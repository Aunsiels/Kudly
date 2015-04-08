#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "usb_serial.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>

// Mailbox for received data
static msg_t mb_buf[32];
MAILBOX_DECL(mb, mb_buf, 32);

static char crlf[] ="\r\n";
static char space[] =" ";

static char wifi_buffer[1];
static char c;

static char cfg_echoOff[] = "set system.cmd.echo off\n\r";
static char cfg_printLevel0[] = "set system.print_level 3\n\r";
static char cfg_promptOff[] = "set system.cmd.prompt_enabled 0\n\r";
static char cfg_headersOn[] = "set system.cmd.header_enabled 1\n\r";

static char ssid[] = "set wlan.ssid \"54vergniaud\"\r\n";
static char passkey[] = "set wlan.passkey \"rose2015rulez\"\r\n";
static char save[] = "save\r\n";
static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";

static char * wifiMessages[] = {
    ssid,
    passkey,
    save,
    http_get,
    stream_read
};
static unsigned int wifiMessagesL = 5;

static enum wifiReadState wifiReadState;

static char feature[25];
static char function[2048];

static EVENTSOURCE_DECL(eventWifiReceptionEnd);
static EVENTSOURCE_DECL(eventWifiSrc);

static SerialConfig uartCfg =
{
    115200,// bit rate
    0,
    0,
    0
};

enum state {
    WAIT_FEATURE,
    WRITE_FEATURE,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_FEATURE
};

void parseXML(char c) {
    static int parse_feature = 0;
    static int parse_function = 0;
    static enum state state = WAIT_FEATURE;

    switch(state) {
        case WAIT_FEATURE:
            if (c == '<') {
                parse_feature = 0;
                state = WRITE_FEATURE;
            }

        case WRITE_FEATURE:
            if (c == '>'){
                state = WAIT_FUNCTION;
                feature[parse_feature] = '\0';
            }
            feature[parse_feature] = c;
            parse_feature++;

        case WAIT_FUNCTION:
            if (c == '<'){
                state = WRITE_FUNCTION;
                parse_function = 0;
            } 

        case WRITE_FUNCTION:
            if (c == '>'){
                state = END_FEATURE;
                function[parse_function-1] = '\0';
                chSysLock();
                chEvtBroadcastI(&eventWifiSrc);
                chSysUnlock();
            }
            function[parse_function]=c;
            parse_function++;

        case END_FEATURE:
            if (c == '>'){
                state = WAIT_FEATURE;
            }
    }
}

void parseLog(char c) {
    (void)c;
}

enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE,
    RECEIVE_LOG
};

static msg_t usartRead_thd(void * args) {
    (void)args;

    static int  h;

    static char header[5];
    static int  headerSize;
    static int  errCode;
    static char rcvType;
    static int  dataCpt;

    while(1) {
        if(chMBFetch(&mb, (msg_t *)&c, TIME_INFINITE) == RDY_OK) {

            switch(wifiReadState) {
                case IDLE:
                    //Message beginning
                    if(c == 'R' || c == 'L' || c == 'S') {
                        wifiReadState = RECEIVE_HEADER;
                        rcvType = c;
                        h = 0;
                    }
                    break;
                case RECEIVE_HEADER:

                    switch(h) {
                        case 0: // Error code
                            errCode = (int)(c - 48);
                            (void)errCode;
                            break;
                        case 1: case 2: case 3: case 4: // Receiving header
                            header[h-1] = c;
                            break;
                        case 5: // Last header character
                            header[h-1] = c;
                            headerSize = strtol(header, (char **)NULL, 10);
                            dataCpt = 0;
                            break;
                        case 7: // After receiving \n\r
                            if(rcvType == 'R') {
                                wifiReadState = RECEIVE_RESPONSE;
                            } else {
                                wifiReadState = RECEIVE_LOG;
                            }
                            break;
                    }
                    
                    h++;
                    break;
                case RECEIVE_RESPONSE:
                    writeSerial("%c", c);
                    parseXML(c);

                    dataCpt++;
                    if(dataCpt == headerSize) {
                        chSysLock();
                        chEvtBroadcastI(&eventWifiReceptionEnd);
                        chSysUnlock();
                        wifiReadState = IDLE;
                    }
                    break;
                case RECEIVE_LOG:
                    writeSerial("%c", c);

                    dataCpt++;
                    if(dataCpt == headerSize) {
                        chSysLock();
                        chEvtBroadcastI(&eventWifiReceptionEnd);
                        chSysUnlock();
                        wifiReadState = IDLE;
                    }
                    break;
            }

        }
    } 
    return 0;
}

static msg_t usartReadInMB_thd(void * args) {
    (void)args;

    while(1) {
        sdRead(&SD3,(uint8_t *) wifi_buffer, 1);
        chMBPost(&mb, wifi_buffer[0], TIME_INFINITE);
    }

    return 0;
}


void wifiInitByUsart(void){  
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));

    sdStart(&SD3, &uartCfg);

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

void wifiWriteByUsart(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length); 
}

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

void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)chp;
  int i;
  for(i = 0; i < argc; i++){
    wifiWriteByUsart(argv[i], strlen(argv[i]));
    wifiWriteByUsart(space, sizeof(space));
  }
  wifiWriteByUsart(crlf, sizeof(crlf));
}


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

void wifiCommands(void) {
  static WORKING_AREA(wifiCommands_wa, 128);
  chThdCreateStatic(
		    wifiCommands_wa, sizeof(wifiCommands_wa),
		    NORMALPRIO, wifiCommands_thd, NULL);
}

void cmdWifiTest(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  wifiWriteByUsart(http_get, sizeof(http_get));
  wifiWriteByUsart(stream_read, sizeof(stream_read));

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

