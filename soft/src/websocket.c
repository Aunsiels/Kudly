#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"
#include "ff.h"

#define WS_DATA_SIZE   32
#define PACKET_SIZE    38 // WS_DATA_SIZE + 6
#define READ_RESP      34 // WS_DATA_SIZE + 2

#define BUFFER_SIZE    64 

#define STR(x) #x
#define STR_(x) STR(x)
#define WEB_SOCKET_MSG "write 0 "STR_(PACKET_SIZE)"\r\n"
#define HEADER_2ND_BYTE 0x80 + WS_DATA_SIZE

static WORKING_AREA(streamingOut_wa, 1024);
static WORKING_AREA(stream_wa, 1024);

/* Codec mailboxes */
static msg_t mbCodecOut_buf[10000];
static msg_t mbCodecIn_buf[10000];
MAILBOX_DECL(mbCodecOut, mbCodecOut_buf, 10000);
MAILBOX_DECL(mbCodecIn, mbCodecIn_buf, 10000);

/* Buffer to send in a websocket */ 
static char codecOutBuffer[BUFFER_SIZE];

/* Streaming event sources */
EventSource streamOutSrc, pollReadSrc;

static char tcpc[] = "tcpc kudly.herokuapp.com 80\r\n";
/*
static char streamWriteHeader[] = "write 0 162\r\n\
GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";
*/
static char streamingWebsocket[] = "write 0 167\r\n\
GET /streaming HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";

static char read400[] = "read 0 400\r\n";
static char readBuffer[] = "read 0 "STR_(BUFFER_SIZE)"\r\n";

// Sending 64 bytes
static char webSocketMsg[PACKET_SIZE + 20];
static char webSocketDataHeader[] = {0x82, HEADER_2ND_BYTE, 0x00, 0x00, 0x00, 0x00};

static bool_t pollRead = FALSE;

static int dataLen;
static int dataCpt;
static int wsHeader;


volatile int websocketSend = 1;
volatile int websocketRecv = 1;

static int poll(void) {
    wifiWriteByUsart("poll 0\n\r", 8);

    if(stream_buffer[0] == '1') {
        return 1;
    } else {
        return 0;
    }
}

/*
 * This function parses websocket data in the buffer
 * It reads the whole buffer
 */
static void parseWebSocketBuffer(void) {
    static int i;
    static int dataStart;
    static uint16_t data;

    i = 0;
    while(i < dataSize - 2) {

        // If new packet comming
        if(wsHeader) {
            /*
             * Data length & data start
             * Stream_buffer[i] is the first byte of header
             * Data length begins on the next byte
             */
            dataLen = stream_buffer[i + 1];
            dataStart = i + 2;

            // length & first data byte position can change...
            if(dataLen == 126) {
                dataLen = (int)(((uint16_t)stream_buffer[i + 2] << 8) | stream_buffer[i + 3]);
                dataStart = i + 4;
            } else if(dataLen == 127) {
            dataLen = (int)(((uint64_t)stream_buffer[i + 9] << 56) |
                                ((uint64_t)stream_buffer[i + 8] << 48) |
                                ((uint64_t)stream_buffer[i + 7] << 40) |
                                ((uint64_t)stream_buffer[i + 6] << 32) |
                                ((uint64_t)stream_buffer[i + 5] << 24) |
                                ((uint64_t)stream_buffer[i + 4] << 16) |
                                ((uint64_t)stream_buffer[i + 3] << 8)  |
                                ((uint64_t)stream_buffer[i + 2]));
                dataStart = i + 10;
            }

            wsHeader = false;
            dataCpt = 0;

            // Dropping the header now that we have all the informations
            i = dataStart;
        } else {

            /*
             * TODO : Do something with the data...
             */
            data = ((uint16_t)(stream_buffer[i+1]) << 8) | (uint16_t)stream_buffer[i];
            //writeSerial("%x", stream_buffer[i+1]);
            //writeSerial("%x", stream_buffer[i]);
            chMBPost(&mbCodecIn, (msg_t)data, TIME_INFINITE);
            i += 2;
            dataCpt += 2;

            if(dataCpt >= dataLen) {
                wsHeader = true;
            }
                
        }

    }
}

static msg_t pollRead_thd(void * args) {
    (void)args;
    EventListener pollReadLst;
    chEvtRegisterMask(&pollReadSrc, &pollReadLst, EVENT_MASK(1));

    while(TRUE) {
        if(chEvtWaitAny(EVENT_MASK(1))) {

            while(poll() != 1) {
                chThdSleepMilliseconds(50);
            }
            wifiWriteByUsart(read400, sizeof(read400));

            pollRead = TRUE;

            // Next packet is the 1st one and starts with a websocket header
            wsHeader = 1;
            writeSerial("Receiving data\n\r");

            while(websocketRecv) {
                while(poll() != 1) {
                    chThdSleepMilliseconds(1);
                }
                wifiWriteByUsart(readBuffer, sizeof(readBuffer));
                parseWebSocketBuffer();
            }

            pollRead = FALSE;
        }
    }

    chEvtUnregister(&pollReadSrc, &pollReadLst);

    return 0;
}

/*
 * Sending data from the codec
 */

static FIL microFile;
static char name[] = "stevie.wav";

static msg_t streamingOut(void * args) {
    (void)args;

    //msg_t msgCodec;
    int lenMsg;
    UINT sizeRead;

    EventListener streamOutLst;
    chEvtRegisterMask(&streamOutSrc, &streamOutLst, (eventmask_t)1);

    writeSerial("Sending thread launched\n\r");

    /*
     * Starting streaming
     */
    if(chEvtWaitAny(1)) {
        if(!f_open(&microFile, name, FA_READ | FA_OPEN_ALWAYS)) {
            writeSerial("Opened !\n\r");
        } else {
            writeSerial("Not opened !\n\r");
        }

        /*
           for(int i = 0 ; i < WS_DATA_SIZE ; i += 2) {
           if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE) == RDY_OK) {
           codecOutBuffer[i]     = (char)(msgCodec >> 8);
           codecOutBuffer[i + 1] = (char)msgCodec;
    //writeSerial("%s", codecOutBuffer[i]);
    }
    }
    */
        while(true) {
            f_read(&microFile, codecOutBuffer, WS_DATA_SIZE, &sizeRead);

            if(WS_DATA_SIZE > (int)sizeRead) {
                break;
            }

            strcat(webSocketMsg, WEB_SOCKET_MSG);
            lenMsg = strlen(webSocketMsg);
            memcpy(&webSocketMsg[lenMsg], webSocketDataHeader, 6);
            memcpy(&webSocketMsg[lenMsg + 6], codecOutBuffer, WS_DATA_SIZE);

            for(int i = 0 ; i < lenMsg + 6 + WS_DATA_SIZE ; i++) {
                writeSerial("%x", webSocketMsg[i]);
            }

            wifiWriteByUsart(webSocketMsg, lenMsg + 6 + WS_DATA_SIZE);
            webSocketMsg[0] = 0;
            // stop sending data, waiting for another event
            if(!websocketSend) {
                continue;
            }

        }

        if(!f_close(&microFile)) {
            writeSerial("Bientot le barbeuc !\n\r");
        } else {
            writeSerial("Fail close...\n\r");
        }

        chEvtUnregister(&streamOutSrc, &streamOutLst);
    }

    chThdSleep(TIME_INFINITE);

    return 0;
}

void parseWebSocket(msg_t data) {
    (void)data;
    /*
    if((char)data == 0xA || (char)data == 0xD)
        writeSerial("%c", (char)data);
    else
        writeSerial("%x", (unsigned char)data);
        */

}

/*
 * Init reading & sending threads
 */
void streamInit(void){

    writeSerial("StreamInit...\n\r");
    chEvtInit(&streamOutSrc);
    chEvtInit(&pollReadSrc);


    chThdCreateStatic(
            streamingOut_wa, sizeof(streamingOut_wa),
            NORMALPRIO, streamingOut, NULL);
   
    chThdCreateStatic(
            stream_wa, sizeof(stream_wa),
            NORMALPRIO, pollRead_thd, NULL);
}

/*
 * Send buffer to wifi module
 */
void sendToWS(void) {
    wifiWriteNoWait(webSocketMsg, sizeof(webSocketMsg) - 1);
    wifiWriteNoWait(webSocketDataHeader, 6);
    wifiWriteNoWait(codecOutBuffer, WS_DATA_SIZE);
}

/*
 * Open a websocket to kudly.herokuapp.com:80/echo
 * Launched by wsinit command
 */
void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

}

/*
 * Unlocks both sending & receiving threads
 * Launched by ws command
 */
void cmdWebSoc(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    writeSerial("Broadcasting...\n\r");
    websocketRecv = 1;
    websocketSend = 1;
    chEvtBroadcast(&streamOutSrc);
    chEvtBroadcast(&pollReadSrc);
}

/*
 * Unlocks receiving thread
 * Thread launched by "stream" command
 */
void cmdDlWave(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    writeSerial("Opening TCP connection & websocket\n\r");
    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamingWebsocket, sizeof(streamingWebsocket));

    chThdSleepMilliseconds(1000);

    wifiWriteByUsart(read400, sizeof(read400));

    writeSerial("Downloading stream...\n\r");
    //websocketRecv = 1;
    websocketSend = 1;
    //chEvtBroadcast(&pollReadSrc);
    chEvtBroadcast(&streamOutSrc);
}

/*
 * Stop receiving
 */
void cmdStopRecv(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    websocketRecv = 0;
}

/*
 * Stop sending
 */
void cmdStopSend(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    websocketSend = 0;
}
