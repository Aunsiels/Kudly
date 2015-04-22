#include "websocket.h"

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "chprintf.h"
#include "wifi.h"
#include "usb_serial.h"
#include "codec.h"
#include "wifi_manager.h"

#define WS_DATA_SIZE   1024

#define BUFFER_SIZE    WS_DATA_SIZE

#define STR(x) #x
#define STR_(x) STR(x)
#define HEADER_2ND_BYTE 0x80 + WS_DATA_SIZE

/* Working areas */
static WORKING_AREA(streamingOut_wa, 1024);
static WORKING_AREA(stream_wa, 1024);

/* Codec mailboxes */
static msg_t mbCodecOut_buf[10000];
static msg_t mbCodecIn_buf[5000];
MAILBOX_DECL(mbCodecOut, mbCodecOut_buf, 10000);
MAILBOX_DECL(mbCodecIn, mbCodecIn_buf, 5000);

/* Buffer to send in a websocket */ 
static char codecOutBuffer[BUFFER_SIZE];

/* Streaming event sources */
EventSource streamOutSrc, pollReadSrc, endStreamEvent;

/* To streaming mode */
static char exitWifi[] = "exit\r\n";
/* Websocket with tcp client */
static char tcpc[] = "tcpc kudly.herokuapp.com 80\r\n";

/* Command Mode */
static char dollar[] = "$$$\r\n";

/* Command list */
static char list[] = "list\r\n";

/* Header for echo */
static char streamWriteHeader[] = "write 0 162\r\n\
GET /echo HTTP/1.1\r\n\
Host: kudly.herokuapp.com\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";

/* Streaming header */
static char downloadWave[] =
"GET /streaming HTTP/1.1\r\n\
Host: 192.168.1.103:9000\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Key: x3JJrRBKLlEzLkh9GBhXDw==\r\n\
Sec-WebSocket-Version: 13\r\n\
\r\n";

/* Read 400 chars command */
static char read400[] = "read 0 400\r\n";

/* Header of the websocket */
static char webSocketDataHeader[] = {0x82, 0xFE, 0x04, 0x00,  0x32, 0x76, 0xA7, 0x3d};

/* Variable to begin or end a send/receive */
volatile int websocketSend = 1;
volatile int websocketRecv = 1;

/*
 * This function parses websocket data in the buffer
 * It reads the whole buffer
 */
static void parseWebSocketBuffer(void) {
    static uint16_t data;
    static msg_t c;
    static char readValue[2];
    static int dataLen;
    static int dataCpt;
    static int wsHeader;

    static int first = 1;

    wsHeader = 1;

    (void)data;


    /* Remove the header */
    while(first) {
        chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
        if((char)c == '\r') {
            chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
            if((char)c == '\n') {
                chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
                if((char)c == '\r') {
                    chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
                    if((char)c == '\n') {
                        break;
                    }
                }
            }
        }
    }

    first = 0;

    writeSerial("\n\r");

    writeSerial("Before\r\n");
    chEvtBroadcast(&streamOutSrc);
    while(websocketRecv) {
        /* TODO Nothing now */
        chThdSleepSeconds(1);
        continue;
        /* If new packet is coming */
        if(wsHeader) {
            chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
            readValue[0] = (char) c;
            if (readValue[0] != 0x82) port_halt();

            /*
             * Data length & data start
             * Stream_buffer[i] is the first byte of header
             * Data length begins on the next byte
             */
	         chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
             readValue[0] = (char) c;

             dataLen = readValue[0];

            /* length & first data byte position can change... */
            if(dataLen == 126) {
                chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
                readValue[0] = (char) c;
                chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
                readValue[1] = (char) c;
                dataLen = (int)(((uint16_t)readValue[0] << 8) | readValue[1]);
            } /* Do not do the last case */
            wsHeader = false;
            dataCpt = 0;
            writeSerial("\n\rdataLen : %d\n\r", dataLen);

        } else {
            /* Read data in the mailbox */
            chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
            readValue[0] = (char) c;
            chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE);
            readValue[1] = (char) c;

            /* Send data to the codec */
            data = ((uint16_t)(readValue[1] << 8) | (uint16_t)readValue[0]);
            chMBPost(&mbCodecIn, (msg_t)data, TIME_INFINITE);
            dataCpt += 2;

            if(dataCpt >= dataLen) {
                wsHeader = true;
            }
                
        }
    }
}

/*
 * Initializes a websocket communication
 */
void webSocketInit(void) {
    writeSerial("Sending websocket request\n\r");

    /* websocket handshake */
    wifiWriteNoWait(downloadWave, sizeof(downloadWave) - 1);
}

/* 
 * Reading thread 
 */
static msg_t pollRead_thd(void * args) {
    (void)args;
    EventListener pollReadLst;
    chEvtRegisterMask(&pollReadSrc, &pollReadLst, (eventmask_t)1);

    chRegSetThreadName("pollread");
    while(TRUE) {
        if(chEvtWaitAny(1)) {
            chMtxLock(&wifiMtx);
            /* Next packet is the 1st one and starts with a websocket header */
            writeSerial("Receiving data\n\r");

            parseWebSocketBuffer();
            chMtxUnlock();
        }
    }

    return 0;
}

/*
 * Sending data from the codec
 */
static msg_t streamingOut(void * args) {
    (void)args;

    msg_t msgCodec;

    chRegSetThreadName("streamingout");
    EventListener streamOutLst;
    chEvtRegisterMask(&streamOutSrc, &streamOutLst, EVENT_MASK(1));

    writeSerial("Sending thread launched\n\r");
    
    while (1) {
        chEvtWaitOne(EVENT_MASK(1));
        writeSerial("After event\r\n");
        while(websocketSend) {
            /* Starting streaming */
            /* Masking */
            webSocketDataHeader[4]++;
            webSocketDataHeader[5]+=11;
            webSocketDataHeader[6]+=32;
            webSocketDataHeader[7]+=3;
            for(int i = 0 ; i < WS_DATA_SIZE ; i += 2) {
                /* Encode data */
                if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE) == RDY_OK) {
                    codecOutBuffer[i]     = ((char)(msgCodec >> 8)) ^
                        webSocketDataHeader[4];
                    codecOutBuffer[i + 1] = ((char)msgCodec) ^
                        webSocketDataHeader[5];
                }
                i+=2;
                if(chMBFetch(&mbCodecOut, &msgCodec, TIME_INFINITE) == RDY_OK) {
                    codecOutBuffer[i]     = ((char)(msgCodec >> 8)) ^
                        webSocketDataHeader[6];
                    codecOutBuffer[i + 1] = ((char)msgCodec) ^
                        webSocketDataHeader[7];
                }      
            }
            /* Websocket header */
            wifiWriteNoWait(webSocketDataHeader, sizeof(webSocketDataHeader));
            /* Send data */
            wifiWriteNoWait(codecOutBuffer, WS_DATA_SIZE);
            chEvtBroadcast(&endStreamEvent);
        }

    }
    return 0;
}

/*
 * Init reading & sending threads
 */
void streamInit(void){

    writeSerial("StreamInit...\n\r");
    chEvtInit(&streamOutSrc);
    chEvtInit(&pollReadSrc);
    chEvtInit(&endStreamEvent);


    chThdCreateStatic(
            streamingOut_wa, sizeof(streamingOut_wa),
            NORMALPRIO + 1, streamingOut, NULL);
   
    chThdCreateStatic(
            stream_wa, sizeof(stream_wa),
            NORMALPRIO, pollRead_thd, NULL);
}

/*
 * Open a websocket to kudly.herokuapp.com:80/echo
 * Launched by wsinit command
 */
void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    wifiWriteByUsart(tcpc, sizeof(tcpc));
    wifiWriteByUsart(streamWriteHeader, sizeof(streamWriteHeader));

    chThdSleepMilliseconds(500);

    wifiWriteByUsart(read400, sizeof(read400));
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

    static int first = 1;

    int error;
    error = wifiWriteByUsart(list, sizeof(list) - 1);
	if(error == 0 || NULL == strstr(stream_buffer, "TCPC")){
        cmdStop(NULL, 0, NULL);
        chMBReset(&mbCodecOut);
        chMBReset(&mbCodecIn);
        wifiWriteNoWait(dollar, sizeof(dollar) -1);
        writeSerial("No connection\r\n");
        return;
    }
    stream_buffer[0] = 0;

    /* To stream mode */
    wifiWriteNoWait(exitWifi,sizeof(exitWifi) - 1);
    streaming = 1;

    chThdSleepMilliseconds(1000);

    if (first)
        webSocketInit();
    first = 0;
    writeSerial("Downloading stream...\n\r");
    websocketRecv = 1;
    websocketSend = 1;
    chEvtBroadcast(&pollReadSrc);
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

/*
 * Stop streaming communication
 */
void cmdStopStream(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    if (!streaming) return;

    websocketSend = 0;
    websocketRecv = 0;
    EventListener el;
    chEvtRegisterMask(&endStreamEvent, &el, EVENT_MASK(1));
    chEvtWaitOne(EVENT_MASK(1));
    cmdStop(NULL, 0, NULL);
    chThdSleepSeconds(10);
    streaming = 0;
    chMBReset(&mbCodecOut);
    chMBReset(&mbCodecIn);
    wifiWriteNoWait(dollar, sizeof(dollar) -1);
    chEvtUnregister(&endStreamEvent, &el);
}
