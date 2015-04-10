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

static FIL fil;
static int writeInFile =0;

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 500\r\n";
static char stream_close[] = "stream_close 0\r\n";

/* Thread that always reads wifi received data */
static msg_t usartRead_thd(void * arg){
    (void)arg;
    static msg_t c;
    static int  h;
    static char header[6];
    static int  headerSize;
    static int  errCode;
    static char rcvType;
    static int  dataCpt;

    static enum wifiReadState wifiReadState = IDLE;

    while(TRUE) {
        if(chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
            /*
             * Parsing headers & data
             */
            switch(wifiReadState) {
	    case IDLE:
		/* Message beginning */
		if((char)c == 'R' || (char)c == 'L' || (char)c == 'S') {
		    wifiReadState = RECEIVE_HEADER;
		    rcvType = (char)c;
		    h = 0;
		}
		break;
	    case RECEIVE_HEADER:
		switch(h) {
		case 0: /* Error code */
		    errCode = (int)((char)c - 48);
		    (void)errCode;
		    break;
		case 1: case 2: case 3: case 4: /* Receiving header */
		    header[h-1] = (char)c; 
		    break;
		case 5: /* Last header character */
		    header[h-1] = (char)c;
		    header[h] = '\0';
		    int i;
		    for(i = 0 ; i < h ; i++){
			if(header[i] != '0')
			    break;
		    }
		    headerSize = strtol(&header[i], (char **)NULL, 10);
		    dataCpt = 0;
		    break;
		case 7: /* After receiving \n\r */
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
		writeSerial("RECEIVE_RESPONSE :%c\r\n",(char)c);
		dataCpt++;
		if(dataCpt == headerSize) {
		    // DO SOMETHING
		    wifiReadState = IDLE;
		}
		break;
	    case RECEIVE_LOG:
		writeSerial("RECEIVE_LOG :%c\r\n",(char)c);
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
    //static EventListener lstEndOfData;
    static FRESULT res;

    (void)res;
    (void)fil;
    //chEvtRegisterMask(&srcEndOfData, &lstEndOfData, 1);
    //   res = f_open(&fil, file, FA_CREATE_ALWAYS | FA_WRITE);
    //if (res)
//	writeSerial("Cannot create this file %d\r\n",res);
    //  else {
	wifiWriteByUsart(http_get, sizeof(http_get));
	//writeSerial("http request\r\n"); 
	chSysLock();
	writeInFile = 1;
	chSysUnlock();
	wifiWriteByUsart(stream_read, sizeof(stream_read));
	//writeSerial("read stream\r\n");
	//chEvtWaitOne(1);
	//writeInFile = 0;
	//writeSerial("broadcast received\r\n");
	wifiWriteByUsart(stream_close, sizeof(stream_close));
//    }
	//f_close(&fil);
	//writeSerial("File closed ");
}
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
