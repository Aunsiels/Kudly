#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"

static char ECHO_1[] = "http_get -o kudly.herokuapp.com/echo\n\r";
static char ECHO_2[] = "had 0 Upgrade websocket\n\r";
static char ECHO_3[] = "had 0 Connection Upgrade\n\r";
static char ECHO_4[] = "had 0 Sec-WebSocket-Key x3JJHMbDL1EzLkh9GBhXDw==\n\r";
static char ECHO_5[] = "had 0 Sec-WebSocket-Protocol chat\n\r";
static char ECHO_6[] = "had 0 Sec-WebSocket-Version 13\n\r";
static char ECHO_7[] = "had 0 Origin http://example.com\n\r";
static char ECHO_8[] = "hre 0\n\r";

/*
 * Initializes a websocket connection
 */
void websocketInit(void){
    /* Init sequence */
    sdWrite(&SD3, (uint8_t*) ECHO_1, sizeof(ECHO_1));
    sdWrite(&SD3, (uint8_t*) ECHO_2, sizeof(ECHO_2));
    sdWrite(&SD3, (uint8_t*) ECHO_3, sizeof(ECHO_3));
    sdWrite(&SD3, (uint8_t*) ECHO_4, sizeof(ECHO_4));
    sdWrite(&SD3, (uint8_t*) ECHO_5, sizeof(ECHO_5));
    sdWrite(&SD3, (uint8_t*) ECHO_6, sizeof(ECHO_6));
    sdWrite(&SD3, (uint8_t*) ECHO_7, sizeof(ECHO_7));
    sdWrite(&SD3, (uint8_t*) ECHO_8, sizeof(ECHO_8));
    /* Read answer */
}

/*
 * Encode a string to send
 */
void websocketEncode(char * str){

    // SEND 129
    int length = strlen(str);

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

void cmdWebSoc(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    static char streamWrite[] = "stream_write 0 5\r\n";
    static char specialChars[] = {0x81, 0x01, 0x65};

    websocketInit();

    chThdSleepMilliseconds(1000);

    wifiWriteByUsart(streamWrite, sizeof(streamWrite));
    wifiWriteByUsart(specialChars, sizeof(specialChars));
    wifiWriteByUsart("\n\r", 2);
}

