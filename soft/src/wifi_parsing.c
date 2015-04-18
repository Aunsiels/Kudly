#include "ch.h"
#include "hal.h"
#include "string.h"
#include "usb_serial.h"
#include "wifi.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <camera.h>

#define FEATURE_SIZE 25
#define FUNCTION_SIZE 2048

/* Event for parsing end */
static EVENTSOURCE_DECL(evtStartSrc);
static EventListener evtStartLst;

/* Feature and function buffer used to launch functionnality by wifi */
static char feature[FEATURE_SIZE];
static char function[FUNCTION_SIZE];

/* State for xml parsing */
enum state {
    WAIT_FEATURE,
    WRITE_FEATURE,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_FEATURE
};
static enum state state = WAIT_FEATURE;

/* Parsing counter */
static int parseFeature;
static int parseFunction;

void parseXML(char c) {

    switch(state) {
    case WAIT_FEATURE:
	/* Wait feature beginning */
	if (c == '<') {
	    parseFeature = 0;
	    state = WRITE_FEATURE;
	}
	break;
    case WRITE_FEATURE:
	/* Save feature name */
	if (c == '>'){
	    state = WAIT_FUNCTION;
	    feature[parseFeature] = '\0';
	}
	feature[parseFeature] = c;
	parseFeature++;
	break;
    case WAIT_FUNCTION:
	/* Wait function beginning */
	if (c == '<'){
	    state = WRITE_FUNCTION;
	    parseFunction = 0;
	} 
	break;
    case WRITE_FUNCTION:
	/* Save function name */
	if (c == '>'){
	    state = END_FEATURE;
	    function[parseFunction-1] = '\0';
	    chEvtBroadcast(&evtStartSrc);
	}
	function[parseFunction]=c;
	parseFunction++;
	break;
    case END_FEATURE:
	/* Wait feature ending */
	if (c == '>'){
	    state = WAIT_FEATURE;
	}
	break;
    }
}


/* Thread waits an wifi event and parse feature and function to launch the rigth function */
static msg_t wifiCommands_thd(void * args) {
    (void)args;  
    chEvtRegisterMask(&evtStartSrc, &evtStartLst, 1);

    while(1) {
	/* Wait for xml ending */
        chEvtWaitOne(1);
	
	/* Led feature */
        if( NULL != strstr(feature,"led")){

	    /* Example : rgb_set n="0" r="125" g="0" b="125" */
            if ( NULL != strstr(function,"rgb_set")){
                int n = strtol(strstr(function,"n=\"") +3,(char **)NULL,10);
                int r = strtol(strstr(function,"r=\"") +3,(char **)NULL,10);
                int g = strtol(strstr(function,"g=\"") +3,(char **)NULL,10);
                int b = strtol(strstr(function,"b=\"") +3,(char **)NULL,10);
                ledSetColorRGB(n, r, g, b);
                continue;
            }

	    /* Example : hsv_set n="0" h="300" s="110" v="49" */
            if ( NULL != strstr(function,"hsv_set")){
                int n = strtol(strstr(function,"n=\"") +3,(char **)NULL,10);
                int h = strtol(strstr(function,"h=\"") +3,(char **)NULL,10);
                int s = strtol(strstr(function,"s=\"") +3,(char **)NULL,10);
                int v = strtol(strstr(function,"v=\"") +3,(char **)NULL,10);
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
