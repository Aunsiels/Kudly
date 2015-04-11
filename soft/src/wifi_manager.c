#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "wifi_manager.h"

enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE
};

static char stream_read[] = "stream_read 0 5000\r\n";
static char stream_close[] ="stream_close 0\r\n";
static char http_get[] ="http_get ";
static char endLine[] ="\r\n";
static bool_t print = TRUE;
static bool_t mailBox = FALSE;

static msg_t mb_buf[320];
MAILBOX_DECL(mbStream, mb_buf, 320);

static FIL fil;
static FRESULT res;

static WORKING_AREA(fileWrite_wa, 128);

static msg_t fileWrite_thd(void * arg){
    (void)arg;
    static msg_t c;
    static char stream_buff[32];
    static int cnt;
    while(TRUE) {
	if(chMBFetch(&mbStream,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
	    stream_buff[cnt] = (char)c;	    
	    if (cnt == 31){
		f_write(&fil, stream_buff, 1, (void*)NULL);
		cnt =0;
	    }
	    else
		cnt++;
	}
    }
    return 0;
}

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
    chThdCreateStatic(
	fileWrite_wa, sizeof(fileWrite_wa),
	NORMALPRIO, fileWrite_thd, NULL);
    
    while(TRUE) {
	    if(chMBFetch(&mbReceiveWifi,(msg_t *)&c,TIME_INFINITE) == RDY_OK){
		 /*
		 * Parsing headers & data
		 */
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
			headerSize = strtol(header, (char **)NULL, 10);
			dataCpt = 0;
			break;
		    case 7: /* After receiving \n\r */
			if(rcvType == 'R') {
			    if(headerSize !=0)
				wifiReadState = RECEIVE_RESPONSE;
			    else{
				externBroadcast();
				wifiReadState = IDLE;
			    }
			} 
			break;
		    }		
		    h++;
		    break;
		case RECEIVE_RESPONSE:
		    if(print){
			writeSerial("%c",(char)c);
		    }
		    if (mailBox){
			chMBPost(&mbStream,(msg_t)c, TIME_INFINITE);
		    }
		    dataCpt++;
		    if(dataCpt == headerSize) {
			// DO SOMETHING    
			externBroadcast();			
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

    f_open(&fil,file,FA_WRITE | FA_CREATE_ALWAYS);
    if (FR_EXIST)
	writeSerial("This file already exist\r\n");
    else if (res) 
        writeSerial("Cannot create this file\r\n");
    
    wifiWriteByUsart(address, strlen(address));
    mailBox = TRUE;
    wifiWriteByUsart(stream_read, sizeof(stream_read));
    mailBox = FALSE;
    wifiWriteByUsart(stream_close,sizeof(stream_close));
    f_close(&fil);
}

void cmdWifiWeb(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    if (argc != 2) {
        writeSerial( "Usage: wifiweb WebAddress SaveLocation\r\n");
        return;
    }
    static char message[120];
    strcat(message ,http_get);
    strcat(message , argv[0]);
    strcat(message , endLine);
    saveWebPage(message, argv[1]);
}



