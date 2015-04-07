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

static char feature[20];
static char function[1048];

static EVENTSOURCE_DECL(eventWifiSrc);

static SerialConfig uartCfg =
{
    115200,// bit rate
    0,
    0,
    0
};

enum state {WAIT_FEATURE, WRITE_FEATURE, WAIT_FUNCTION, WRITE_FUNCTION, END_FEATURE};

static msg_t usartRead_thd(void * args) {
    (void)args;
    int parse_feature = 0;
    int parse_function = 0;
    enum state state = WAIT_FEATURE;
    while(1) {
      if(chMBFetch(&mb, (msg_t *)&c, TIME_INFINITE) == RDY_OK) {
	//writeSerial("%c", c);
	
	if (state == WAIT_FEATURE){
	  if (c == '<') {
	    parse_feature = 0;
	    state = WRITE_FEATURE;
	    continue;
	  }
	}
	
	if (state == WRITE_FEATURE){
	  if (c == '>'){
	    state = WAIT_FUNCTION;
	    feature[parse_feature] = '\0';
	    continue;
	  }
	    feature[parse_feature] = c;
	    parse_feature++;
	}
	
	if (state == WAIT_FUNCTION){
	  if (c == '<'){
	    state = WRITE_FUNCTION;
	    parse_function = 0;
	    continue;
	  } 
	}

	if (state == WRITE_FUNCTION){
	  if (c == '>'){
	    state = END_FEATURE;
	    function[parse_function-1] = '\0';
	    chSysLock();
	    chEvtBroadcastI(&eventWifiSrc);
	    chSysUnlock();
	    continue;
	  }
	    function[parse_function]=c;
	    parse_function++;
	}
	
	if (state == END_FEATURE){
	  if (c == '>'){
	    state = WAIT_FEATURE;
	    continue;
	  }
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


static msg_t wifiCommands_thd(void * args) {
  (void)args;
  
  EventListener eventWifiLst;
  chEvtRegisterMask(&eventWifiSrc, &eventWifiLst, 1);
  
  while(1) {
    chEvtWaitOne(1);
    if( NULL != strstr(feature,"led")){
      char* ptr;
      int n = strtol(strstr(function,"n=\"") +3,&ptr,10);
      int r = strtol(strstr(function,"r=\"") +3 ,&ptr,10);
      int g = strtol(strstr(function,"g=\"") +3,&ptr,10);
      int b = strtol(strstr(function,"b=\"") +3,&ptr,10);
      ledSetColorRGB(n, r, g, b);
      continue;
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