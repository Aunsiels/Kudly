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

char wifi_buffer[1];
char c;
char led_rgb[50];

static SerialConfig uartCfg =
{
    115200,// bit rate
    0,
    0,
    0
};

static msg_t usartRead_thd(void * args) {
    (void)args;
    int cnt_rgb = 0;
    int i = 0;
    while(1) {
        if(chMBFetch(&mb, (msg_t *)&c, TIME_INFINITE) == RDY_OK) {
	  //writeSerial("%c", c);
	  /*
	   * Send byte to the codec, the SD card...
	   */
	  if(cnt_rgb == 4){
	    if(c == ';') {
	      cnt_rgb = 0;
	      i = 0;
	      char * ptr;
	      int r = strtol(led_rgb , &ptr , 10);
	      int g = strtol(ptr , &ptr , 10);
	      int b = strtol(ptr , &ptr , 10);;
	      ledSetColorRGB(0,r,g,b);
	      writeSerial("done with : %d,%d,%d \r\n", r,g,b);
	    }	
	    else{
	      led_rgb[i] = c;
	      i++;
	    }
	  }
	  else{
	    if (c == 'r') 
	      cnt_rgb = 1;
	    if (c == 'g') 
	      cnt_rgb = (cnt_rgb == 1) ? cnt_rgb + 1 : 0;
	    if (c == 'b') 
	      cnt_rgb = (cnt_rgb == 2) ? cnt_rgb + 1 : 0;
	    if (c == '=') 
	      cnt_rgb = (cnt_rgb == 3) ? cnt_rgb + 1 : 0;

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

