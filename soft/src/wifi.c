#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
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
static char c;

/* Some string used by initialization to configure network */
static char ssid[] = "set wlan.ssid \"54vergniaud\"\r\n";
static char passkey[] = "set wlan.passkey \"rose2015rulez\"\r\n";
static char save[] = "save\r\n";

/* http request on Kudly website */
static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";

/* Feature and function buffer used to launch functionnality by wifi */
static char feature[20];
static char function[1048];

/* Event source to signal a that wifi receive a function and feature */
static EVENTSOURCE_DECL(eventWifiSrc);

/* Serial driver that uses usart3 */
static SerialConfig uartCfg =
{
    115200,
    0,
    0,
    0
};

/* States for parsing state machine */
enum state {WAIT_FEATURE, WRITE_FEATURE, WAIT_FUNCTION, WRITE_FUNCTION, END_FEATURE};

/* Thread that always reads wifi received data */
static msg_t usartRead_thd(void * args) {
    (void)args;
    /* Index to fill feature and fucntion tabs */ 
    int parse_feature = 0;
    int parse_function = 0;
    
    /* Default state */
    enum state state = WAIT_FEATURE;

    while(1) {
      if(chMBFetch(&mb, (msg_t *)&c, TIME_INFINITE) == RDY_OK) {
	//writeSerial("%c", c);
	
	/* Wait for feature frame beginning */
	if (state == WAIT_FEATURE){
	  if (c == '<') {
	    parse_feature = 0;
	    state = WRITE_FEATURE;
	    continue;
	  }
	}
	
	/* Write feature frame in feture buffer */
	if (state == WRITE_FEATURE){
	  if (c == '>'){
	    state = WAIT_FUNCTION;
	    feature[parse_feature] = '\0';
	    continue;
	  }
	    feature[parse_feature] = c;
	    parse_feature++;
	}
	
	/* Wait for function frame beginning */
	if (state == WAIT_FUNCTION){
	  if (c == '<'){
	    state = WRITE_FUNCTION;
	    parse_function = 0;
	    continue;
	  } 
	}

	/* Write function in frame buffer */
	if (state == WRITE_FUNCTION){
	  if (c == '>'){
	    state = END_FEATURE;
	    function[parse_function-1] = '\0';
	    /* Function and feature are ready : send event broadcast */
	    chSysLock();
	    chEvtBroadcastI(&eventWifiSrc);
	    chSysUnlock();
	    continue;
	  }
	    function[parse_function]=c;
	    parse_function++;
	}
	
	/* Wait for feature frame end */
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
    wifiWriteByUsart(ssid, sizeof(ssid));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(passkey, sizeof(passkey));
    chThdSleepMilliseconds(1000);
    wifiWriteByUsart(save, sizeof(save));
    chThdSleepMilliseconds(1000);
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
}
