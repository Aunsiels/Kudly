#include "websocket.c"

#include "hal.h"
#include "ch.h"
#include "string.h"

#define ECH0_1 "http_get -o kudly.herokuapp.com/echo"
#define ECHO_2 "had 0 Upgrade websocket"
#define ECHO_3 "had 0 Connection Upgrade"
#define ECHO_4 "had 0 Sec-WebSocket-Key x3JJHMbDL1EzLkh9GBhXDw=="
#define ECHO_5 "had 0 Sec-WebSocket-Protocol chat"
#define ECHO_6 "had 0 Sec-WebSocket-Version 13"
#define ECHO_7 "had 0 Origin http://example.com"
#define ECHO_8 "hre 0"

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

