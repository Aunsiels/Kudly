#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "wifi_manager.h"

/* Different states for usart reading */
enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE
};

/* Some strings used by http_request and stream reding */
static char stream_read[] = "stream_read 0 200\r\n";
static char http_get[] ="http_get ";
static char endLine[] ="\r\n";

/* String used to build http request request */
static char msgWifi[120];

/* Boolean for printing and saving usart data */
static bool_t print = TRUE;
static bool_t save = FALSE;

/* For system file */
static FIL fil;
static FRESULT res;

/* Array where data received are saving */
static char stream_buffer[203];

/* Event source to signal whan all data are received */
EVENTSOURCE_DECL(srcEndToReadUsart);

/* Data size to be used after broacast */
static int dataSize;

/* Thread that always reads wifi received data */
static msg_t usartRead_thd(void * arg){
    (void)arg;
    static msg_t c;
    static int  h;
    static char header[6];
    static int  errCode;
    static char rcvType;
    static int  dataCpt;   
    static int  headerSize;
    static enum wifiReadState wifiReadState = IDLE;    

    while(TRUE) {
	if(chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
	    /* Parsing headers & data */
	    switch(wifiReadState) {	    
	    case IDLE:
		/* Message beginning */
		if((char)c == 'R') {
		    wifiReadState = RECEIVE_HEADER;
		    rcvType = (char)c;
		    h = 0;
		}
		break;

	    case RECEIVE_HEADER:
		/* Header beginning */
		switch(h) {
		case 0:
		    /* Error code */
		    errCode = (int)((char)c - '0');
		    (void)errCode;
		    break;
		case 1: case 2: case 3: case 4:
		    /* Receiving header */
		    header[h-1] = (char)c; 
		    break;
		case 5:
		    /* Last header character */
		    header[h-1] = (char)c;
		    header[h] = '\0';
		    headerSize = strtol(header, (char **)NULL, 10);
		    dataCpt = 0;
		    break;
		case 7: 
		    /* After receiving \n\r */
		    if(rcvType == 'R') {
			if(headerSize !=0)
			    wifiReadState = RECEIVE_RESPONSE;
			else{
			    chEvtBroadcast(&srcEndToReadUsart);
			    wifiReadState = IDLE;
			}
		    } 
		    break;
		}		
		h++;
		break;
		
	    case RECEIVE_RESPONSE:
		/* Response beginning */
		/* Printing on shell */
		if(print)
		    writeSerial("%c",(char)c);
		/* Saving in stream_buffer */
		if (save)
		    stream_buffer[dataCpt]= (char)c;

		dataCpt++;

		/* End of stream_buffer - dataSize updating */
		if(dataCpt == headerSize) {
		    stream_buffer[dataCpt]='\0';
		    dataSize = headerSize;
		    chEvtBroadcast(&srcEndToReadUsart);
		    wifiReadState = IDLE;
		}
		break;
	    default :
		break;
	    }
	}
    }
    return 0;
}

/* Launch wifi recv parsing thread above */
void usartRead(void) {
    static WORKING_AREA(usartRead_wa, 2048);

    chThdCreateStatic(
	usartRead_wa, sizeof(usartRead_wa),
	NORMALPRIO, usartRead_thd, NULL);
}
 
/* Function that sends hhtp_request and save th page in file */
static void saveWebPage( char * address , char * file){
    
    res = f_open(&fil,file,FA_WRITE | FA_CREATE_ALWAYS);
    
    if (res == FR_EXIST)
	writeSerial("This file already exist\r\n");
    else if (res) 
        writeSerial("Cannot create this file\r\n");
    else {
	/* Send http_request */
	wifiWriteByUsart(address, strlen(address));
	save = TRUE;
	print = FALSE;
	/* Read the first stream */
	wifiWriteByUsart(stream_read, sizeof(stream_read));
	/* Read until stream is not closed */
	while (NULL == strstr(stream_buffer, "Command failed")){
	    f_write(&fil,stream_buffer,dataSize-2,(void*)NULL);
	    wifiWriteByUsart(stream_read, sizeof(stream_read));
	}
	save = FALSE;
	print =TRUE;
	writeSerial("Webpage saved\r\n");
    }
    f_close(&fil);
}

/* Function that sends hhtp_request and save th page in file */
void cmdWifiWeb(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    if (argc != 2) {
        writeSerial( "Usage: wifiweb WebAddress SaveLocation\r\n");
        return;
    }
    strcat(msgWifi ,http_get);
    strcat(msgWifi , argv[0]);
    strcat(msgWifi , endLine);
    saveWebPage(msgWifi, argv[1]);
    msgWifi[0] ='\0';
}
