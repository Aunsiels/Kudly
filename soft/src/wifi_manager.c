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

//static char stream_read[] = "stream_read 0 5000\r\n";



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
		writeSerial("Case : %d %c\r\n",wifiReadState,(char)c);
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
		    //writeSerial("%c",(char)c);
		    dataCpt++;
		    if(dataCpt == headerSize) {
			// DO SOMETHING    
			writeSerial("broadcast\r\n");
			
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
    (void) file;
    (void)address;
//    wifiWriteByUsart(address, strlen(address));
    //  wifiWriteByUsart(stream_read, sizeof(stream_read));
}

void cmdWifiWeb(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    (void)argc;
    (void)argv;
    saveWebPage("http_get kudly.herokuapp.com/pwm\r\n", "wifi.txt");
}
