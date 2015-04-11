#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"

static char tcpc[] = "tcpc kudly.herokuapp.com 80\r\n";
static char streamWrite[] = "write 0 284\r\n";

static char webSocketClient1[] =
"GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Pragma: no-cache\r\n\
Cache-Control: no-cache\r\n\
Upgrade: websocket\r\n";

static char webSocketClient2[] =
"Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJHRbDL1EzLkh9GBhXDw==\r\n\
Sec-WebSocket-Protocol: chat, superchat\r\n\
Sec-WebSocket-Version: 13\r\n\
Origin: http://kudly.herokuapp.com\r\n\
\r\n";

/*
 * Initializes a websocket connection
 */
void websocketInit(void){
    /* Init sequence */
    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamWrite, sizeof(streamWrite));
    wifiWriteByUsart(webSocketClient1, sizeof(webSocketClient1));
    wifiWriteByUsart(webSocketClient2, sizeof(webSocketClient2));
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

    websocketInit();

}

