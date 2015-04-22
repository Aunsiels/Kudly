#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include "wifi_manager.h"
#include "wifi_parsing.h"

/* Event listener for the end of reading by usart */ 
static EventListener lstEndToReadUsart;

/* Mailbox for received data */
static msg_t mb_buf[32];
MAILBOX_DECL(mbReceiveWifi, mb_buf, 32);

/* Special strings to print */
static char crlf[] ="\r\n";
static char space[] =" ";
static char cmdMessage[120];

/* Char and buffer used by wifi receive */
static char wifi_buffer;

/* Some string used by initialization to configure network */
static char ssid[] = "set wlan.ssid \"54vergniaud\"\r\n";
static char passkey[] = "set wlan.passkey \"rose2015rulez\"\r\n";
static char save[] = "save\r\n";

/* http request on Kudly website */
static char cfg_echoOff[] = "set system.cmd.echo off\r\n";
static char cfg_printLevel0[] = "set system.print_level 0\r\n";
static char cfg_promptOff[] = "set system.cmd.prompt_enabled 0\r\n";
static char cfg_headersOn[] = "set system.cmd.header_enabled 1\r\n";

/* String for lowpower mode */
static char wakeUp[] = "set system.wakeup.events gpio22\r\n";
static char sleep[] ="sleep\r\n";

/* Mutex for wifi access */
Mutex wifiAccessMtx;
Mutex writeMtx;

/* Serial driver that uses usart3 */
static SerialConfig uartCfg =
{
    115200,
    0,
    0,
    0
};

/* Thread that reads wifi data and puts it on Mailbox */
static msg_t usartReadInMB_thd(void * args) {
    (void)args;

    while(1) {
        sdRead(&SD3,(uint8_t *) &wifi_buffer, 1);
        chMBPost(&mbReceiveWifi,(msg_t)wifi_buffer, TIME_INFINITE);
    }
    return 0;
}

/* Sends data by wifi */
void wifiWriteByUsart(char * message, int length){
    chMtxLock(&writeMtx);    
    chEvtRegisterMask(&srcEndToReadUsart, &lstEndToReadUsart,EVENT_MASK(1));
    sdWrite(&SD3, (uint8_t*)message, length);
    chEvtWaitOne(EVENT_MASK(1));
    chEvtUnregister(&srcEndToReadUsart, &lstEndToReadUsart);
    chMtxUnlock();
}

/* Same as above but don't want to wait for the response */
void wifiWriteNoWait(char * message, int length){
    chMtxLock(&writeMtx);
    sdWrite(&SD3, (uint8_t*)message, length);
    chMtxUnlock();
}

/*  Launches the wifi reading */
static void wifiReadByUsart(void) {
    static WORKING_AREA(usartReadInMB_wa, 2048);
    
    chThdCreateStatic(
	usartReadInMB_wa, sizeof(usartReadInMB_wa),
	NORMALPRIO, usartReadInMB_thd, NULL);
}

/* Command shell to speak with wifi module in command mode */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    int i;
    for(i = 0; i < argc; i++){
	strcat(cmdMessage ,argv[i]);
	strcat(cmdMessage ,space);
    }
    strcat(cmdMessage ,crlf);
    wifiWriteByUsart(cmdMessage, strlen(cmdMessage));
    cmdMessage[0]='\0';
}

/* Initialization of wifi network */
void wifiInitByUsart(void) {
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));
    palSetPadMode (GPIOD,GPIOD_WIFI_WAKEUP, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad (GPIOD,GPIOD_WIFI_WAKEUP);
   
    /* Init Mutex for wifi access */
    chMtxInit(&wifiAccessMtx);
    chEvtInit(&srcEndToReadUsart);
    chMtxInit(&writeMtx);

    /* Start usart 3 */

    sdStart(&SD3, &uartCfg);
    /* Fill mailbox with usart data */
    wifiReadByUsart();
    /* Send commad to wifi module */
    wifiWriteByUsart(cfg_echoOff, sizeof(cfg_echoOff)-1);
    wifiWriteByUsart(cfg_printLevel0, sizeof(cfg_printLevel0)-1);
    wifiWriteByUsart(cfg_headersOn, sizeof(cfg_headersOn)-1);
    wifiWriteByUsart(cfg_promptOff, sizeof(cfg_promptOff)-1);
    wifiWriteByUsart(wakeUp, sizeof(wakeUp)-1);
    wifiWriteByUsart(ssid, sizeof(ssid)-1);
    wifiWriteByUsart(passkey, sizeof(passkey)-1);
    wifiWriteByUsart(save, sizeof(save)-1);
    
    /* Loop to test the network connection */
    bool_t state;
    while(TRUE){
	state = wifiNup();
	if (state == TRUE){
	    ledSetColorRGB(0, 0, 255, 0);
	    break;
	}
	else
	    ledSetColorRGB(0, 255, 0, 0);

	chThdSleepMilliseconds(500);
	ledSetColorRGB(0, 0, 0, 0);
	chThdSleepMilliseconds(500);
    }
    chThdSleepMilliseconds(500);
    ledSetColorRGB(0, 0, 0, 0);
    writeSerial("Wifi ready to use\r\n");
    /* Active Thread that wait xml command */
    wifiCommands();

    /*
     * Configuring wifi module in machine friendly command mode
     * cf : http://wiconnect.ack.me/2.1/serial_interface#configuration
     */
}


static msg_t wifiSleep(void){
    writeSerial("Wifi is sleeping\r\n");	
    wifiWriteByUsart(sleep, sizeof(sleep)-1);
    return 0;
}

static void wifiWakeUp(void){
    palSetPad (GPIOD,GPIOD_WIFI_WAKEUP);    
    chThdSleepMilliseconds(100);
    palClearPad (GPIOD,GPIOD_WIFI_WAKEUP);    
    writeSerial("Wifi is woke up\r\n");
}

/* Command to sleep the wifi module */
void cmdWifiSleep(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    wifiSleep();
}

/* Command to wake up the wifi module */
void cmdWifiWakeUp(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    wifiWakeUp();
}
