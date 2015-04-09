#include "ch.h"
#include "hal.h"
#include "string.h"
#include "usb_serial.h"
#include "wifi.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>

static EVENTSOURCE_DECL(eventWifiSrc);

/* Feature and function buffer used to launch functionnality by wifi */
static char feature[25];
static char function[2048];

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";
static char stream_close[] = "stream_close all\r\n";

enum state {
    WAIT_FEATURE,
    WRITE_FEATURE,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_FEATURE
};

/* Command shell to send http request and read the stream */
void cmdWifiTest(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;
    wifiWriteByUsart(http_get, sizeof(http_get));
    wifiWriteByUsart(stream_read, sizeof(stream_read));
    wifiWriteByUsart(stream_close, sizeof(stream_close));

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
