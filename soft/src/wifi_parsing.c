#include "ch.h"
#include "hal.h"
#include "string.h"
#include "usb_serial.h"
#include "wifi.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include "camera.h"
#include "application.h"

#define CONFIG_SIZE 25
#define FUNCTION_SIZE 2048

/*Event for xml functions */
EVENTSOURCE_DECL(eventPhotoSrc);
EVENTSOURCE_DECL(eventStreamSrc);
EVENTSOURCE_DECL(eventSoundSrc);
EVENTSOURCE_DECL(eventStorySrc);
EVENTSOURCE_DECL(eventGameSrc);

/* Config and function buffer used to launch functionnality by wifi */
static char config[CONFIG_SIZE];
static char function[FUNCTION_SIZE];

/* State for xml parsing */
enum state {
    WAIT_CONFIG,
    WRITE_CONFIG,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_CONFIG
};
static enum state state = WAIT_CONFIG;

/* Parsing counter */
static int parseConfig;
static int parseFunction;
static int stateXML;

/* Thread waits an wifi event and parse feature and function to launch the rigth function */
static void wifiCommands(void) {
    /* Photo */
    if ( NULL != strstr(function,"photo")) {
        stateXML = strtol(strstr(function,"state=\"") +7,(char **)NULL,10);
        if(stateXML) {
            chEvtBroadcast(&eventPhotoSrc);
        }
        return;
    }
    /* Story */
    if ( NULL != strstr(function,"story")) {
        stateXML = strtol(strstr(function,"state=\"") +7,(char **)NULL,10);
        if(stateXML) {
            chEvtBroadcast(&eventStorySrc);
        }
        return;
    }
    /* Sound */
    if ( NULL != strstr(function,"sound")) {
        stateXML = strtol(strstr(function,"state=\"") +7,(char **)NULL,10);
        if(stateXML) {
            chEvtBroadcast(&eventSoundSrc);
        }
        return;
    }

    /* Stream */
    if ( NULL != strstr(function,"stream")) {
        stateXML = strtol(strstr(function,"state=\"") +7,(char **)NULL,10);
        if(stateXML) {
            chEvtBroadcast(&eventStreamSrc);
        }
        return;
    }

    /* Game */
    if ( NULL != strstr(function,"game")) {
        stateXML = strtol(strstr(function,"state=\"") +7,(char **)NULL,10);
        if(stateXML) {
            chEvtBroadcast(&eventGameSrc);
        }
        return;
    }
}

void parseXML(char c) {
    switch(state) {
    case WAIT_CONFIG:
        /* Wait feature beginning */
        if (c == '<') {
            parseConfig = 0;
            state = WRITE_CONFIG;
        }
        break;
    case WRITE_CONFIG:
        /* Save feature name */
        if (c == '>') {
            state = WAIT_FUNCTION;
            config[parseConfig] = '\0';
        }
        config[parseConfig] = c;
        parseConfig++;
        break;
    case WAIT_FUNCTION:
        /* Wait function beginning */
        if (c == '<') {
            state = WRITE_FUNCTION;
            parseFunction = 0;
        }
        break;
    case WRITE_FUNCTION:
        /* Save function name */
        if (c == '/' && parseFunction == 0) {
            state = END_CONFIG;
            break;
        }
        if (c == '>') {
            state = WAIT_FUNCTION;
            function[parseFunction-1] = '\0';
            wifiCommands();
        }
        function[parseFunction]=c;
        parseFunction++;
        break;
    case END_CONFIG:
        /* Wait feature ending */
        if (c == '>') {
            state = WAIT_CONFIG;
        }
        break;
    }
}
