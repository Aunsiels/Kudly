#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>

/* Mailbox for received data */
static msg_t mb_buf[32];
MAILBOX_DECL(mb, mb_buf, 32);

/* Special strings to print */
static char crlf[] ="\r\n";
static char space[] =" ";

/* Char and buffer used by wifi receive */
static char wifi_buffer[1];

/* Some string used by initialization to configure network */
static char ssid[] = "set wlan.ssid \"54vergniaud\"\r\n";
static char passkey[] = "set wlan.passkey \"rose2015rulez\"\r\n";
static char save[] = "save\r\n";
static char nup[] = "nup\r\n";
static char gpio0[] = "gdi 0 none\r\ngdi 0 ood\r\n";

/* http request on Kudly website */
static char cfg_echoOff[] = "set system.cmd.echo off\n\r";
static char cfg_printLevel0[] = "set system.print_level 3\n\r";
static char cfg_promptOff[] = "set system.cmd.prompt_enabled 0\n\r";
static char cfg_headersOn[] = "set system.cmd.header_enabled 1\n\r";

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";
static char stream_close[] = "stream_close all\r\n";

/* Feature and function buffer used to launch functionnality by wifi */
static char feature[25];
static char function[2048];

/* Event source to signal a that wifi receive a function and feature */
static int wifiStream; // Shared variable to notify end of stream

static char * wifiMessages[] = {
    ssid,
    passkey,
    save,
    http_get,
    stream_read
};
static unsigned int wifiMessagesL = 5;

static EVENTSOURCE_DECL(eventWifiReceptionEnd);
static EVENTSOURCE_DECL(eventWifiReceivedLog);
static EVENTSOURCE_DECL(eventWifiSrc);

/* Serial driver that uses usart3 */
static SerialConfig uartCfg =
{
    115200,
    0,
    0,
    0
};

void parseLog(char c) {
    (void)c;
}

/* Thread that reads wifi data and puts it on Mailbox */
static msg_t usartReadInMB_thd(void * args) {
    (void)args;

    while(1) {
        sdRead(&SD3,(uint8_t *) wifi_buffer, 1);
        chMBPost(&mb, wifi_buffer[0], TIME_INFINITE);
    }

    return 0;
}

/* Initialization of wifi network */
void wifiInitByUsart(void){  
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));

    sdStart(&SD3, &uartCfg);
    wifiWriteByUsart(gpio0, sizeof(gpio0));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(ssid, sizeof(ssid));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(passkey, sizeof(passkey));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(save, sizeof(save));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(nup, sizeof(nup));

    /*
     * Configuring wifi module in machine friendly command mode
     * cf : http://wiconnect.ack.me/2.1/serial_interface#configuration
     */
    wifiWriteByUsart(cfg_echoOff, sizeof(cfg_echoOff));
    wifiWriteByUsart(cfg_printLevel0, sizeof(cfg_printLevel0));
    wifiWriteByUsart(cfg_headersOn, sizeof(cfg_headersOn));
    wifiWriteByUsart(cfg_promptOff, sizeof(cfg_promptOff));
    wifiReadByUsart();

}

static msg_t usartRead_thd(void * arg){
    (void)arg;
    char c;
    while(TRUE){
	if(chMBFetch(&mb,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
	    writeSerial("%c",c);
	}
    }
    return 0;
}

/* Sends data by wifi */
void wifiWriteByUsart(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length); 
}

/*  Launches the wifi reading */
void wifiReadByUsart(void) {
    static WORKING_AREA(usartRead_wa, 128);
    static WORKING_AREA(usartReadInMB_wa, 128);

    chThdCreateStatic(
            usartReadInMB_wa, sizeof(usartReadInMB_wa),
            NORMALPRIO, usartReadInMB_thd, NULL);

    chThdCreateStatic(
            usartRead_wa, sizeof(usartRead_wa),
            NORMALPRIO, usartRead_thd, NULL);
}

/* Command shell to speak with wifi module in command mode */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)chp;
  int i;
  for(i = 0; i < argc; i++){
    wifiWriteByUsart(argv[i], strlen(argv[i]));
    wifiWriteByUsart(space, sizeof(space));
  }
  wifiWriteByUsart(crlf, sizeof(crlf));
}

/* Thread waits an wifi event and parse feature and function to launch the rigth function */
static msg_t wifiCommands_thd(void * args) {
  (void)args;  
  EventListener eventWifiLst;
  chEvtRegisterMask(&eventWifiSrc, &eventWifiLst, 1);
  char* ptr;  
  while(1) {
    chEvtWaitOne(1);
    if( NULL != strstr(feature,"led")){
      if ( NULL != strstr(function,"rgb_set")){
	int n = strtol(strstr(function,"n=\"") +3,&ptr,10);
	int r = strtol(strstr(function,"r=\"") +3,&ptr,10);
	int g = strtol(strstr(function,"g=\"") +3,&ptr,10);
	int b = strtol(strstr(function,"b=\"") +3,&ptr,10);
	ledSetColorRGB(n, r, g, b);
      continue;
      }
      
      if ( NULL != strstr(function,"hsv_set")){	
	int n = strtol(strstr(function,"n=\"") +3,&ptr,10);
	int h = strtol(strstr(function,"h=\"") +3,&ptr,10);
	int s = strtol(strstr(function,"s=\"") +3,&ptr,10);
	int v = strtol(strstr(function,"v=\"") +3,&ptr,10);
	ledSetColorHSV(n, h, s, v);
      continue;
      }
    }
  }
  return 0;
}


/* Launches thread that wait wifi event */
void wifiCommands(void) {
  static WORKING_AREA(wifiCommands_wa, 128);
  chThdCreateStatic(
		    wifiCommands_wa, sizeof(wifiCommands_wa),
		    NORMALPRIO, wifiCommands_thd, NULL);
}

/* Command shell to send http request and read the stream */
void cmdWifiTest(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  wifiWriteByUsart(http_get, sizeof(http_get));
  wifiWriteByUsart(stream_read, sizeof(stream_read));
  wifiWriteByUsart(stream_close, sizeof(stream_close));
  
}

static msg_t wifiSendMessages_thd(void * args) {
    (void)args;

    for(unsigned int i = 0 ; i < wifiMessagesL ; i++) {
        wifiWriteByUsart(wifiMessages[i], sizeof(wifiMessages[i]));
    }

    chThdSleep(TIME_INFINITE);

    return 0;
} 

void sendMessages(void) {
    static WORKING_AREA(wifiSendMessages_wa, 128);

    chThdCreateStatic(
            wifiSendMessages_wa, sizeof(wifiSendMessages_wa),
            NORMALPRIO, wifiSendMessages_thd, NULL);
}


static msg_t wifiDLFile_thd(void * args) {
    (void)args;

    static char dlRequest[] = "http_get kudly.herokuapp.com\n\r";
    static char streamRead[] = "stream_read 0 5\n\r"; 

    wifiWriteByUsart(dlRequest, sizeof(dlRequest));
    wifiStream = 1;

    while(true) {
        wifiWriteByUsart(streamRead, sizeof(streamRead));
        if(!wifiStream) {
            break;
        }
    }

    return 0;
}

static msg_t receivedLog_thd(void * args) {
    (void)args;

    EventListener eventWifiReceivedLogLst;
    chEvtRegisterMask(&eventWifiReceivedLog, &eventWifiReceivedLogLst, 1);

    while(true) {
        if(chEvtWaitOne(1)) {
            writeSerial("Log treated");
            wifiStream = 0;
        }
    }
    return 0;
}

void cmdWifiDL(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    static WORKING_AREA(receivedLog_wa, 128);
    static WORKING_AREA(wifiDLFile_wa, 128);

    chThdCreateStatic(
            receivedLog_wa, sizeof(receivedLog_wa),
            NORMALPRIO, receivedLog_thd, chp);

    chThdCreateStatic(
            wifiDLFile_wa, sizeof(wifiDLFile_wa),
            NORMALPRIO, wifiDLFile_thd, NULL);

}
