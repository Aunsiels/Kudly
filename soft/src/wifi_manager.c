#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"

enum wifiReadState {
IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE,
    RECEIVE_LOG
    };

static EVENTSOURCE_DECL(srcEndOfData);

//static FIL fil;
//static UINT* bw;
static int writeInFile =0;

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 500\r\n";
static char stream_close[] = "stream_close all\r\n";

/* Thread that always reads wifi received data */
static msg_t usartRead_thd(void * arg){
    (void)arg;
    static char c;
    static int  h;
    static char header[5];
    static int  headerSize;
    static int  errCode;
    static char rcvType;
    static int  dataCpt;

    static enum wifiReadState wifiReadState;

    while(TRUE) {
        if(chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
            /*
             * Parsing headers & data
             */
            switch(wifiReadState) {
	    case IDLE:
		//Message beginning
		if(c == 'R' || c == 'L' || c == 'S') {
		    wifiReadState = RECEIVE_HEADER;
		    rcvType = c;
		    h = 0;
		}
		break;
	    case RECEIVE_HEADER:
		switch(h) {
		case 0: // Error code
		    errCode = (int)(c - 48);
		    (void)errCode;
		    break;
		case 1: case 2: case 3: case 4: // Receiving header
		    header[h-1] = c;
		    break;
		case 5: // Last header character
		    header[h-1] = c;
		    headerSize = strtol(header, (char **)NULL, 10);
		    dataCpt = 0;
		    break;
		case 7: // After receiving \n\r
		    if(rcvType == 'R') {
			wifiReadState = RECEIVE_RESPONSE;
		    } else {
			wifiReadState = RECEIVE_LOG;
		    }
		    break;
		}
		
		h++;
		break;
	    case RECEIVE_RESPONSE:
		if(writeInFile){
//		    static FRESULT res;
		    //res = f_write(&fil,&c,1,bw);
		    writeSerial("Data %c\r\n", c);
		    // if (*bw != 1)
		    //	writeSerial("Error: write 0 byte\r\n");
		    //if (res)
		    //	writeSerial("Cannot write one byte on file\r\n");
		}
		dataCpt++;
		writeSerial("Data %c\r\n", c);
		if(dataCpt >= headerSize) {
		    if(writeInFile){
			chEvtBroadcast(&srcEndOfData);
		    }
		    wifiReadState = IDLE;
		}
		break;
	    case RECEIVE_LOG:        
		dataCpt++;
		if(dataCpt == headerSize) {
		    // DO SOMETHING
		    wifiReadState = IDLE;
		}
		break;
            }
        }
    } 
    return 0;
}

/*
 * Launch wifi recv parsing thread above
 */
void usartRead(void) {
    static WORKING_AREA(usartRead_wa, 128);

    chThdCreateStatic(
	usartRead_wa, sizeof(usartRead_wa),
	NORMALPRIO, usartRead_thd, NULL);
}

void saveWebPage( char * address , char * file){
    (void)address;
    (void)file;
    static EventListener lstEndOfData;
    //static FRESULT res;

    chEvtRegisterMask(&srcEndOfData, &lstEndOfData, 1);
//    res = f_open(&fil, file, FA_CREATE_ALWAYS | FA_WRITE);
    //  if (res)
//	writeSerial("Cannot create this file %d\r\n",res);
    //  else {
	wifiWriteByUsart(http_get, sizeof(http_get));
	writeSerial("http request\r\n"); 
	writeInFile = 1;
	wifiWriteByUsart(stream_read, sizeof(stream_read));
	writeSerial("read stream\r\n");
	//chEvtWaitAny(1);
	//writeInFile = 0;
      	writeSerial("broadcast received\r\n");
	wifiWriteByUsart(stream_close, sizeof(stream_close));
	// }
	// f_close(&fil);
    writeSerial("File closed ");
/*
 * Streaming through socket
 */
void cmdWifiStream(BaseSequentialStream * chp, int argc, char * argv[]) {
    (void)chp;
    (void)argc;
    (void)argv;

    static char testHello[] = "Salut ! Ça va bien ?\r\n";

    static char streamMode[] = "set bus.mode stream\n\r";
    static char tcpAuto[] = "set tcp.client.auto_start 1\n\r";
    static char tcpCli[] = "set tcp.client.remote_host 137.194.43.247\n\r";
    static char tcpPort[] = "set tcp.client.remote_port 6789\n\r";
    static char saveReboot[] = "save\n\rreboot\n\r";

    writeSerial("Configuring to stream mode...\n\r");
    wifiWriteByUsart(streamMode, sizeof(streamMode));
    wifiWriteByUsart(tcpCli, sizeof(tcpCli));
    wifiWriteByUsart(tcpPort, sizeof(tcpPort));
    wifiWriteByUsart(tcpAuto, sizeof(tcpAuto));
    wifiWriteByUsart(saveReboot, sizeof(saveReboot));

    chThdSleepMilliseconds(1000);

    writeSerial("Starting streaming...\n\r");
    for(int i = 0 ; i < 10 ; i++) {
        wifiWriteByUsart(testHello, sizeof(testHello));
        chThdSleepMilliseconds(500);
    }
}
