#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"

static char STREAM_WRITE[] = "stream_write 0 ";

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
static char streamWriteHeader[] = "write 0 163\r\n\
GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";

static msg_t streamingIn(void * args) {
    (void)args;

    EventListener streamInLst;
    chEvtRegisterMask(&streamInSrc, &streamInLst, (eventmask_t)1);


    return 0;
}


static msg_t streamingOut(void * args) {
    (void)args;

    //static msg_t msgCodec;

    EventListener streamOutLst;
    chEvtRegisterMask(&streamOutSrc, &streamOutLst, (eventmask_t)1);
    
    while(true) {
        /*
         * Starting streaming
         */
        if(chEvtWaitAny(1)) {
            while(true) {
                for(int i = 0 ; i < 16 ; i++) {
                    codecOutBuffer[i] = (char)(41 + i);
                    /*
                    if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE)) {
                        codecOutBuffer[i] = (char)msgCodec;
                    }
                    */
                }

                chThdSleepMilliseconds(500);

                sendToWS(codecOutBuffer);
            }
        }
    }

    chThdSleep(TIME_INFINITE);

    return 0;
}

void streamLaunch(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    streamInit();
    chEvtBroadcast(&streamOutSrc);
}

/*
 * Initializes a websocket connection
 */
void streamInit(void){

    /* Websocket init sequence */
    wifiWriteByUsart(tcpc, sizeof(tcpc));
    writeSerial("-\n\r", 5);
    wifiWriteByUsart(streamWriteHeader, sizeof(streamWriteHeader));
    writeSerial("--\n\r", 5);

    // TODO : wait for an event when receiving all data
    chThdSleepMilliseconds(500);

    wifiWriteByUsart("read 0 200\r\n", 11);

    // TODO : wait for an event when receiving all data
    chThdSleepMilliseconds(500);

    static WORKING_AREA(streamingOut_wa, 128);
    static WORKING_AREA(streamingIn_wa, 128);

    /* Events init */
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
    static char webSocketMsg[] = "write 0 22\r\n......dddddddddddddddd";
    static char webSocketDataHeader[] = {0x81, 0x88, 0x00, 0x00, 0x00, 0x00};

    strcpy(&webSocketMsg[12], webSocketDataHeader);
    strcpy(&webSocketMsg[18], str);

    // Sending message header + 16 bytes = 22 bytes
    wifiWriteByUsart(webSocketMsg, sizeof(webSocketMsg));

    // TODO : really necessary ??
    chThdSleepMilliseconds(500);

    wifiWriteByUsart("read 0 10\r\n", 11);
    */
}

/*
 * Encode a string to send
 */
void websocketEncode(char * str){
    int length = strlen(str);

    /* The length of the message to send */
    int msgLength = 0;
    /* Message type */
    msgLength++;
    /* Size of the message encoder */
    if (length < 126) {
        msgLength++;
    } else if (length < 65536) {
        msgLength += 3;
    } else {
        msgLength += 9;
    }
    /* Raw message */
    msgLength += length;

    /* Begins to send message */
    sdWrite(&SD3, (uint8_t*) STREAM_WRITE, sizeof(STREAM_WRITE));

    // SEND 129

    if (length < 126) {
        //send (char) length
    } else if (length < 65536) {
        //send 126
        //send (char) ((length >> 8) & 255)
        //send (char) (length & 255)
    } else {
        //send 127
        //send (char) (length >> 56 & 255)
        //send (char) (length >> 48 & 255)
        //send (char) (length >> 40 & 255)
        //send (char) (length >> 32 & 255)
        //send (char) (length >> 24 & 255)
        //send (char) (length >> 16 & 255)
        //send (char) (length >>  8 & 255)
        //send (char) (length       & 255)
    }

    int i;
    for(i = 0; i < length; ++i){
        //send str[i]
    }
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

    if(argc == 0) {
        chprintf(chp, "Écrit 8 chars après la commande steuplai\r\n");
        return;
    }

    sendToWS(argv[0]);
}

