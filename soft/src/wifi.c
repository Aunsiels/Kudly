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
static char wifi_buffer;

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
        chMBPost(&mb, wifi_buffer, TIME_INFINITE);
	writeSerial("%c", wifi_buffer);
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



/* Sends data by wifi */
void wifiWriteByUsart(char * message, int length){
    sdWrite(&SD3, (uint8_t*)message, length); 
}

/*  Launches the wifi reading */
void wifiReadByUsart(void) {
    static WORKING_AREA(usartReadInMB_wa, 128);

    chThdCreateStatic(
            usartReadInMB_wa, sizeof(usartReadInMB_wa),
            NORMALPRIO, usartReadInMB_thd, NULL);
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
