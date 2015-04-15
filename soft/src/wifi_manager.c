#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "wifi_manager.h"
#include "websocket.h"
#include "wifi_parsing.h"

/* !!! dataRead (for stream_read command) must be greather than dataWrite (for stream_write command)*/
#define dataRead 500
#define dataWrite 32 

/* Different states for usart reading */
enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE
};

/* Boolean to set streaming to on */
bool_t streaming = FALSE;

/* Some string for polling functions */
static char stream_poll[] = "stream_poll 0\r\n";
static char stream_close[] = "stream_close all\r\n";
static char command_failed [] = "Command failed";

/* Some strings used by http get request and stream reading */
static char stream_read[] = "stream_read 0 500\r\n";
static char http_get[] ="http_get ";
static char endLine[] ="\r\n";

/* Some strings used by http_post */
static char http_post[]="http_post ";
static char urlencoded[]=" x-www-form-urlencoded\r\n";

/* String used to build http get request */
static char msgWifi[120];

/* Boolean for printing and saving usart data */
bool_t print = TRUE;
bool_t save = TRUE;

/* For system file */
static FIL fil;
static FRESULT res;

/* Array where data received are saving */
char stream_buffer[dataRead + 4];

/* Some string for uploading */
static char file_create[] = "file_create -o ";
static DWORD dword;
static char itoaBuff[10];
static char stream_write[] ="stream_write 0 ";
static char writeBuff[dataWrite + 1];
static char http_upload[] = "http_upload ";
static char file_delete [] = "file_delete ";

static int msgWifiLen;

UINT br;

/* Event source to signal whan all data are received */
EVENTSOURCE_DECL(srcEndToReadUsart);

/* Data size to be used after broacast */
int dataSize;

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
		    dataCpt = 0;
		    stream_buffer[0]= '\0';
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
		    /* Add end string character to print */
		    header[h] = '\0';
		    headerSize = strtol(header, (char **)NULL, 10);
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
		    writeSerial("%c",(unsigned char)c);
		/* Saving in stream_buffer */
		if (save)
		    stream_buffer[dataCpt]= (char)c;
		dataCpt++;

		/* End of stream_buffer - dataSize updating */
		if(dataCpt == headerSize) {
		    /* Add end string character to print */
		    if (save)
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

/* Polling for http_get command */ 
static void polling(void){
    systime_t time = chTimeNow();

    wifiWriteByUsart(stream_poll, sizeof(stream_poll));

    /* Wait for data or if buffer (resp. 1) is empty (resp. Command failed)*/
    while(TRUE){

	/* Stop the function when while is looping for 1 second to stop polling*/
	if (chTimeNow() - time > MS2ST(1000)){
	    wifiWriteByUsart(stream_close, sizeof(stream_close));
	    writeSerial( "Timeout : buffer is empty\r\n");
	    break;
	}
	
	if(NULL != strstr(stream_buffer, command_failed) || NULL != strstr(stream_buffer, "1"))
	    break;
	else {
	    wifiWriteByUsart(stream_poll, sizeof(stream_poll));
	}
    }
}

/* Function that sends hhtp_request and save th page in file */
static void saveWebPage( char * address , char * file){
    
    /* Build http request command */
    strcat(msgWifi , http_get);
    strcat(msgWifi , address);
    strcat(msgWifi , endLine);
    
    /* Send http_request */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';

    /* Create file and open it with writing mode */ 
    res = f_open(&fil,file,FA_WRITE | FA_CREATE_NEW);
    if (res == FR_EXIST)
	writeSerial("This file already exist\r\n");
    else if (res) 
        writeSerial("Cannot create this file\r\n");
    else {
	save = TRUE;
	print = FALSE;
	
	/* Read the first stream if available */
	polling();
	wifiWriteByUsart(stream_read, sizeof(stream_read));

	/* Read until stream is not closed */
	while (NULL == strstr(stream_buffer, command_failed)){
	    f_write(&fil,stream_buffer,dataSize-2,(void*)NULL);
	    polling();    
	    wifiWriteByUsart(stream_read, sizeof(stream_read));	
	}
	save = FALSE;
	print =TRUE;
	writeSerial("Webpage saved\r\n");
    }
    f_close(&fil);
}

/* Function that sends hhtp_request and save th page in file */
void cmdWifiGet(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    if (argc != 2) {
        writeSerial("Usage: uploadwifi <web address> <local file>\r\n");
        return;
    }
    saveWebPage(argv[0], argv[1]);
}

/* Function that sends hhtp_request and save th page in file */
static void postAndRead( char * address , char * data){

    /* Build hhtp post request */
    strcat(msgWifi ,http_post);
    strcat(msgWifi , address);
    strcat(msgWifi , "?");
    strcat(msgWifi, data);
    strcat(msgWifi , urlencoded);

    /* Send http_post */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));

    msgWifi[0] ='\0';

    /* Read the first stream */
    print = FALSE;
    save = TRUE;
    
    /* Read the first stream if available */
    polling();
    wifiWriteByUsart(stream_read, sizeof(stream_read));

    /* Read until stream is not closed */
    while (NULL == strstr(stream_buffer, command_failed)){
	writeSerial(stream_buffer);
	polling();
   	wifiWriteByUsart(stream_read, sizeof(stream_read));
    }
    
    save = FALSE;
    print = TRUE;
    writeSerial("Response received\r\n");
}

/* Function that sends hhtp_post and save th page in file */
void cmdWifiPost(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    if (argc != 2) {
        writeSerial("Usage: uploadwifi <web address> <data>\r\n");
        return;
    }
    postAndRead(argv[0],argv[1]);
}

static void uploadFile( char *address , char * localFile , char * remoteFile){
   
    /* Open file in reading mode */
    res = f_open(&fil,localFile,FA_OPEN_EXISTING | FA_READ);
    if (res) {
        writeSerial("Cannot read this file %d\r\n",res);
        f_close(&fil);
	return;
    }

    /* Get the file size */
    dword = f_size(&fil);

    /* Build wifi module command to create a new file */
    strcat(msgWifi ,file_create);
    strcat(msgWifi ,localFile);
    strcat(msgWifi , " ");
    itoa(dword , itoaBuff,10);
    strcat(msgWifi ,itoaBuff);
    strcat(msgWifi ,endLine);
   
    /* Send command to create a file */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';

    print = FALSE;
    save = TRUE;

    /* Read file in SD Card and send data to wifi module */ 
    while(TRUE){
	
	/* Fille buffer with file's data */
	res = f_read(&fil, writeBuff, dataWrite ,&br );
	if(res){
	    writeSerial("Error when reading file %d", res);
	    break;
	}

	/* Build string to write data in stream */
	strcat(msgWifi,stream_write);
	itoa(br, itoaBuff,10);
	strcat(msgWifi,itoaBuff);
	strcat(msgWifi,endLine);

	/* Add data in the end of the string */
	msgWifiLen = strlen(msgWifi);                       
	for(int i = 0; i<(int)br;i++){
	    msgWifi[msgWifiLen+i] = writeBuff[i];
	}

        /* Send wifi command to write in file */
	wifiWriteByUsart(msgWifi, msgWifiLen+br);
	
	if(NULL == strstr(stream_buffer, "Success")){
	    writeSerial("A stream not sent\r\n");
	    break;
	} 
	msgWifi[0] ='\0';
	
	if(br != dataWrite){	    
	    break;
	}
    }

    writeSerial("The file is in the wifi module memory\r\n");
    res = f_close(&fil);

    /* Build string to upload file */
    strcat(msgWifi,http_upload);
    strcat(msgWifi,address);
    strcat(msgWifi," ");
    strcat(msgWifi,localFile);
    strcat(msgWifi," ");
    strcat(msgWifi,remoteFile);
    strcat(msgWifi,endLine);

    /* Send wifi command to write in file */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));

    msgWifi[0] ='\0';
    
    writeSerial("File sent correctly\r\n");
    
    print = TRUE;
    save = FALSE;
    
    /* Build string to delete file in wifi module flash */
    strcat(msgWifi,file_delete);
    strcat(msgWifi,localFile);
    strcat(msgWifi,endLine);

    /* Send wifi command to delete file in wifi module flash */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));    
    msgWifi[0] ='\0';   
    
    writeSerial("File deleted in wifi module flash\r\n");
}

/* Shell command to upload a file in SD card on server */
void cmdWifiUpload(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    if (argc != 3) {
        writeSerial("Usage: uploadwifi <web address> <local file> <remote file>\r\n");
        return;
    }
    uploadFile( argv[0] , argv[1] , argv[2]); 
}

/* Function that sends hhtp_request and send data to xml parsing */
static void parsePage( char * address){

    /* Build http request command */
    strcat(msgWifi , http_get);
    strcat(msgWifi , address);
    strcat(msgWifi , endLine);

    /* Send wifi command to get page */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';
    
    /* Read the first stream */
    print = FALSE;
    save = TRUE;

    /* Read the first stream if available */
    polling();
    wifiWriteByUsart(stream_read, sizeof(stream_read));

    /* Read until stream is not closed */
    while (NULL == strstr(stream_buffer, command_failed)){
	/* Send each character */
	for (int i = 0 ; i < dataSize ; i++){
	    parseXML(stream_buffer[i]);
	}
	polling();
   	wifiWriteByUsart(stream_read, sizeof(stream_read));
    }
    save = FALSE;
    print = TRUE;
    
    writeSerial("Page read\r\n");
}

/* Shell command to read a web xml page on server and execute actions */
void cmdWifiXml(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)chp;
    if (argc != 1) {
        writeSerial("Usage: parsewifi <web address>\r\n");
        return;
    }
    parsePage(argv[0]); 
}


