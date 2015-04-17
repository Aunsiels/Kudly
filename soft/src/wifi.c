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
static char nup[] = "nup\r\n";
static char save[] = "save\r\n";

/* http request on Kudly website */
static char cfg_echoOff[] = "set system.cmd.echo off\r\n";
static char cfg_printLevel0[] = "set system.print_level 0\r\n";
static char cfg_promptOff[] = "set system.cmd.prompt_enabled 0\r\n";
static char cfg_headersOn[] = "set system.cmd.header_enabled 1\r\n";

/* Wifi command to set it in high speed */
static char uart_baud [] = "set ua b 0 921600\r\n";
static char uart_flow [] = "set ua f 0 on\r\n";
static char reboot [] = "reboot\r\n";

/* Serial driver that uses usart3 */
static SerialConfig uartCfgHigh =
{
    921600,
    0,
    0,
    USART_CR3_CTSE | USART_CR3_RTSE
};

/* Serial driver that uses usart3 */
static SerialConfig uartCfgLow =
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
    chEvtRegisterMask(&srcEndToReadUsart, &lstEndToReadUsart,1);
    sdWrite(&SD3, (uint8_t*)message, length);
    chEvtWaitOne(1);
    chEvtUnregister(&srcEndToReadUsart, &lstEndToReadUsart);
}

/* Sends data by wifi */
void wifiWriteByUsartNoWait(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length);
}

/* Same as above but don't want to wait for the response */
void wifiWriteNoWait(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length);
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

    chThdSleepMilliseconds(10000);

    sdStart(&SD3, &uartCfgLow);
    writeSerial("1\r\n");

    wifiWriteByUsartNoWait(cfg_echoOff, sizeof(cfg_echoOff));
    wifiWriteByUsartNoWait(cfg_printLevel0, sizeof(cfg_printLevel0));
    wifiWriteByUsartNoWait(cfg_headersOn, sizeof(cfg_headersOn));
    wifiWriteByUsartNoWait(cfg_promptOff, sizeof(cfg_promptOff));
    wifiWriteByUsartNoWait(ssid, sizeof(ssid));
    wifiWriteByUsartNoWait(passkey, sizeof(passkey));
    wifiWriteByUsartNoWait(uart_baud, sizeof(uart_baud));
    wifiWriteByUsartNoWait(uart_flow, sizeof(uart_flow));
    wifiWriteByUsartNoWait(save, sizeof(save));
    wifiWriteByUsartNoWait(reboot, sizeof(reboot));

    chThdSleepMilliseconds(5000);
    writeSerial("2\r\n");
    sdStop(&SD3);
    writeSerial("3\r\n");
    sdStart(&SD3, &uartCfgHigh);
    writeSerial("4\r\n");

    /* Read wifi by usart */
    usartRead();

    wifiReadByUsart();
    wifiWriteByUsart(ssid, sizeof(ssid));
    wifiWriteByUsart(passkey, sizeof(passkey));
    wifiWriteByUsart(save, sizeof(save));

    wifiWriteByUsart(nup, sizeof(nup));
    writeSerial("5\r\n");
      
    chThdSleepMilliseconds(4000);
    wifiWriteByUsart(nup, sizeof(nup));
    writeSerial("Wifi ready to use\r\n");
    /*
     * Configuring wifi module in machine friendly command mode
     * cf : http://wiconnect.ack.me/2.1/serial_interface#configuration
     */
}
