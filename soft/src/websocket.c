#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"

static char STREAM_WRITE[] = "stream_write 0 ";

/* Codec mailboxes */
//static msg_t mbCodecOut_buf[32];
//static msg_t mbCodecIn_buf[32];
//MAILBOX_DECL(mbCodecOut, mcCodecOut_buf, 32);
//MAILBOX_DECL(mbCodecIn, mcCodecIn_buf, 32);

/* Contains an int to send */
//static char intStringSend[10];

static char tcpc[] = "tcpc kudly.herokuapp.com 80\r\n";
static char streamWrite[] = "write 0 163\r\n";

static char webSocketHeader[] =
"GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";

/*
 * Initializes a websocket connection
 */
void websocketInit(void){
    //static WORKING_AREA(streamingOut_wa, 128);
    //static WORKING_AREA(streamingIn_wa, 128);

    /* Init sequence */
    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamWrite, sizeof(streamWrite));
    wifiWriteByUsart(webSocketHeader, sizeof(webSocketHeader));
    chThdSleepMilliseconds(500);

    wifiWriteByUsart("read 0 200\r\n", 11);
    chThdSleepMilliseconds(500);

    /*
    chThdCreateStatic(
            streamingOut_wa, sizeof(streamingOut_wa),
            NORMALPRIO, streamingOut, NULL);

    chThdCreateStatic(
            streamingOut_wa, sizeof(streamingOut_wa),
            NORMALPRIO, streamingOut, NULL);
            */
}

void sendToWS(char * str) {
    static char webSocketData[] = {0x81, 0x88, 0x00, 0x00, 0x00, 0x00};

    wifiWriteByUsart("write 0 14\r\n", 12);
    wifiWriteByUsart(webSocketData, 6);
    wifiWriteByUsart(str, 8);

    chThdSleepMilliseconds(500);
    wifiWriteByUsart("read 0 10\r\n", 11);

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

    websocketInit();

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

