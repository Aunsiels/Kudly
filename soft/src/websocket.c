#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"

/* Codec mailboxes */
static msg_t mbCodecOut_buf[32];
static msg_t mbCodecIn_buf[32];
MAILBOX_DECL(mbCodecOut, mbCodecOut_buf, 32);
MAILBOX_DECL(mbCodecIn, mbCodecIn_buf, 32);

/* Buffer to send in a websocket */ 
static char codecOutBuffer[16];

/* Streaming event sources */
EventSource streamOutSrc, streamInSrc;

static char tcpc[] = "tcpc kudly.herokuapp.com 80\r\n";
static char streamWriteHeader[] = "write 0 162\r\n\
GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";
static char webSocketMsg[] = "write 0 22\r\nhhhhhhdddddddddddddddd";
static int webSocketMsgSize = sizeof(webSocketMsg) - 1;
static char webSocketDataHeader[] = {0x81, 0x90, 0x00, 0x00, 0x00, 0x00};

msg_t streamingIn(void * args) {
    (void)args;

    EventListener streamInLst;
    chEvtRegisterMask(&streamInSrc, &streamInLst, (eventmask_t)1);
    writeSerial("Reading...\n\r");

    while(true) {
        /*
         * Starting streaming
         */
        if(chEvtWaitAny(1)) {
            while(true) {
                wifiWriteByUsart("read 0 18\n\r", 11);
                chThdSleepMilliseconds(2000);
            }
        }
    }

    return 0;
}

msg_t streamingOut(void * args) {
    (void)args;

    msg_t msgCodec;

    EventListener streamOutLst;
    chEvtRegisterMask(&streamOutSrc, &streamOutLst, (eventmask_t)1);

    chThdSleepMilliseconds(3000);
    
    while(true) {
        /*
         * Starting streaming
         */
        if(chEvtWaitAny(1)) {
            while(true) {
                for(int i = 0 ; i < 8 ; i++) {
                    if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE) == RDY_OK) {
                        codecOutBuffer[i]     = (char)(msgCodec >> 8);
                        codecOutBuffer[i + 1] = (char)msgCodec;
                        writeSerial("%c%c",
                                codecOutBuffer[i],
                                codecOutBuffer[i+1]);
                    }
                }
                writeSerial("\n\r");
                sendToWS(codecOutBuffer);

                //Counter for speedtest
                chThdSleepMilliseconds(500);
            }
        }
    }

    chThdSleep(TIME_INFINITE);

    return 0;
}

void parseStreamData(msg_t c) {
    writeSerial("r:%c", (char)c);

    /*
    if(cpt >= 2) {
        //writeSerial("%c", (char)c);
        //chMBFetch(&mbCodecIn, c, TIME_INFINITE);
    }

    cpt++;
    */
}

void streamLaunch(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    streamInit();
    chEvtBroadcast(&streamOutSrc);
    chEvtBroadcast(&streamInSrc);
}

/*
 * Initializes a websocket connection
 */
void streamInit(void){

    /* Websocket init sequence */
    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamWriteHeader, sizeof(streamWriteHeader));

    chThdSleepMilliseconds(500);

    wifiWriteByUsart("read 0 400\r\n", 12);

    chThdSleepMilliseconds(500);

    chEvtInit(&streamOutSrc);
    chEvtInit(&streamInSrc);

    //streaming = 1;
    //print = 0;
    
    static WORKING_AREA(streamingOut_wa, 128);
    static WORKING_AREA(streamingIn_wa, 128);

    // Events init 

    chThdCreateStatic(
            streamingIn_wa, sizeof(streamingIn_wa),
            NORMALPRIO, streamingIn, NULL);

    chThdCreateStatic(
            streamingOut_wa, sizeof(streamingOut_wa),
            NORMALPRIO, streamingOut, NULL);
   
}

void sendToWS(char * str) {
    (void)str;
    /*
     * Header length = 6
     * Data length   = 16
     */
    memcpy(&webSocketMsg[12], webSocketDataHeader, 6);
    memcpy(&webSocketMsg[18], str, 16);

    wifiWriteByUsart(webSocketMsg, webSocketMsgSize);
    wifiWriteByUsart("read 0 18\n\r", 11);
}

void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    streamInit();

}

void cmdWebSoc(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    writeSerial("Broadcasting...\n\r");
    chEvtBroadcast(&streamOutSrc);
    /*
    if(argc == 0) {
        chprintf(chp, "Écrit 8 chars après la commande steuplai\n\r");
        return;
    }

    sendToWS(argv[0]);
    */
}

