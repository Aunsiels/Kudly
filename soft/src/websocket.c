#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"

#define WS_DATA_SIZE   64
#define PACKET_SIZE    70 // WS_DATA_SIZE + 6
#define READ_RESP      66 // WS_DATA_SIZE + 2

#define STR(x) #x
#define STR_(x) STR(x)
#define WEB_SOCKET_MSG "write 0 "STR_(PACKET_SIZE)"\r\n"
#define HEADER_2ND_BYTE 0x80 + WS_DATA_SIZE

static WORKING_AREA(streamingOut_wa, 128);
static WORKING_AREA(streamingIn_wa, 128);

/* Codec mailboxes */
static msg_t mbCodecOut_buf[256];
static msg_t mbCodecIn_buf[256];
MAILBOX_DECL(mbCodecOut, mbCodecOut_buf, 128);
MAILBOX_DECL(mbCodecIn, mbCodecIn_buf, 128);

/* Buffer to send in a websocket */ 
static char codecOutBuffer[WS_DATA_SIZE];

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
static char read400[] = "read 0 400\r\n";

// Sending 64 bytes
static char webSocketMsg[] = WEB_SOCKET_MSG;
static char webSocketDataHeader[] = {0x82, HEADER_2ND_BYTE, 0x00, 0x00, 0x00, 0x00};

msg_t streamingIn(void * args) {
    (void)args;

    EventListener streamInLst;
    chEvtRegisterMask(&streamInSrc, &streamInLst, (eventmask_t)1);
    writeSerial("Reading thread launched\n\r");

    while(true) {
        // Starting streaming
        if(chEvtWaitAny(1)) {
            for(int j = 0 ; j < 200 ; j++) {
                chThdSleepMilliseconds(60);
                wifiWriteByUsart("read 0 "STR_(READ_RESP)"\n\r", 12);
            }
        }
    }

    chThdSleep(TIME_INFINITE);

    return 0;
}

msg_t streamingOut(void * args) {
    (void)args;

    msg_t msgCodec;

    EventListener streamOutLst;
    chEvtRegisterMask(&streamOutSrc, &streamOutLst, (eventmask_t)1);

    writeSerial("Sending thread launched\n\r");
    
    while(true) {
        /*
         * Starting streaming
         */
        if(chEvtWaitAny(1)) {
            for(int j = 0 ; j < 100 ; j++) {
                for(int i = 0 ; i < WS_DATA_SIZE ; i += 2) {
                    if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE) == RDY_OK) {
                        codecOutBuffer[i]     = (char)(msgCodec >> 8);
                        codecOutBuffer[i + 1] = (char)msgCodec;
                    }
                }
                sendToWS(codecOutBuffer);

                chThdSleepMilliseconds(50);
            }
        }
    }

    chThdSleep(TIME_INFINITE);

    return 0;
}

void parseStreamData(msg_t data) {
    (void)data;

    writeSerial("%c", (char)data);
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
 * Init reading & sending threads
 */
void streamInit(void){


    chEvtInit(&streamOutSrc);
    chEvtInit(&streamInSrc);

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
    wifiWriteNoWait(webSocketMsg, sizeof(webSocketMsg) - 1);
    wifiWriteNoWait(webSocketDataHeader, 6);
    wifiWriteNoWait(codecOutBuffer, WS_DATA_SIZE);
}

void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamWriteHeader, sizeof(streamWriteHeader));

    chThdSleepMilliseconds(500);

    wifiWriteByUsart(read400, sizeof(read400));
}

void cmdWebSoc(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    writeSerial("Broadcasting...\n\r");
    chEvtBroadcast(&streamOutSrc);
    chEvtBroadcast(&streamInSrc);
    /*
    if(argc == 0) {
        chprintf(chp, "Écrit 8 chars après la commande steuplai\n\r");
        return;
    }

    sendToWS(argv[0]);
    */
}

