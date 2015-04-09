#include "ch.h"
#include "hal.h"
#include "string.h"
#include "usb_serial.h"
#include <stdio.h>
#include <stdlib.h>

static enum wifiReadState wifiReadState;

static EVENTSOURCE_DECL(eventWifiSrc);
static EVENTSOURCE_DECL(eventWifiReceptionEnd);
static EVENTSOURCE_DECL(eventWifiReceivedLog);

/* Feature and function buffer used to launch functionnality by wifi */
static char feature[25];
static char function[2048];

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";
static char stream_close[] = "stream_close all\r\n";

/* Event source to signal a that wifi receive a function and feature */
static int wifiStream; // Shared variable to notify end of stream

enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE,
    RECEIVE_LOG
};

enum state {
    WAIT_FEATURE,
    WRITE_FEATURE,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_FEATURE
};

void parseLog(char c) {
    (void)c;
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

/* Command shell to send http request and read the stream */
void cmdWifiTest(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
  wifiWriteByUsart(http_get, sizeof(http_get));
  wifiWriteByUsart(stream_read, sizeof(stream_read));
  wifiWriteByUsart(stream_close, sizeof(stream_close));
  
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


/* Thread that always reads wifi received data */
void wifiMsgParsing(char c) {
    static int  h;
    static char header[5];
    static int  headerSize;
    static int  errCode;
    static char rcvType;
    static int  dataCpt;
    
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
	    writeSerial("LOG !\n\r");
	    chSysLock();
	    chEvtBroadcastI(&eventWifiReceivedLog);
	    chSysUnlock();
	    wifiReadState = IDLE;
	}
	break;
    }
    
    return 0;
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
