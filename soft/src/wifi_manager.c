#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"
#include "wifi_manager.h"
#include "wifi_parsing.h"

/* !!! dataRead (for stream_read command) must be greather than dataWrite (for stream_write command)*/
#define DATA_READ 1440
#define STR(x) #x
#define STR_(x) STR(x)
#define STREAM_READ "stream_read 0 "STR_(DATA_READ)"\r\n"
#define DATA_WRITE 1440 

/* Different states for usart reading */
enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE
};

/* Some string for polling functions */
static char stream_poll[] = "stream_poll ";
static char stream_close[] = "stream_close ";
static char command_failed [] = "Command failed";

/* Some strings used by http get request and stream reading */
static char stream_read[] = "stream_read ";
static char http_get[] ="http_get ";
static char endLine[] ="\r\n";

/* Some strings used by http_post */
static char http_post[]="http_post ";
static char urlencoded[]=" x-www-form-urlencoded\r\n";
static int dataRead = DATA_READ;

/* String used to build http get request */
static char msgWifi[DATA_READ + 4];

/* Boolean for printing and saving usart data */
bool_t print = TRUE;
bool_t save = TRUE;

/* For system file */
static FIL fil;
static FRESULT res;

/* Array where data received are saving */
static char stream_buffer[DATA_READ + 4];

/* Some string for uploading */
static char file_create[] = "file_create -o ";
static DWORD dword;
static char itoaBuff[10];
static char stream_write[] ="stream_write ";
static char writeBuff[DATA_WRITE + 4];
static char http_upload[] = "http_upload ";
static char file_delete [] = "file_delete ";

UINT br;

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
			    dataSize = headerSize;
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

static void sendStreamCommand (char * command, int * stream , int *number , char * data){
	msgWifi[0] ='\0';
	strcat(msgWifi , command);
	itoa(*stream , itoaBuff,10);
	strcat(msgWifi ,itoaBuff);
	
	if (number != NULL) { 
	    strcat(msgWifi ," ");
	    itoa(*number , itoaBuff,10);
	    strcat(msgWifi ,itoaBuff);
	}
	strcat(msgWifi , endLine);
	
	int msgWifiLen = strlen(msgWifi);
		
	if (data != NULL){    
	    for (int i = 0 ; i < *number; i++){
		msgWifi[ msgWifiLen + i ] = data[i];
	    } 	 
	    wifiWriteByUsart(msgWifi, msgWifiLen + *number);
	    msgWifi[0] ='\0';
	}
	else{
	    wifiWriteByUsart(msgWifi, msgWifiLen);
	    msgWifi[0] ='\0';
	}
}
/* Polling for http_get command */ 
static void polling(int * stream){
    systime_t time = chTimeNow();

    sendStreamCommand(stream_poll, stream , NULL , NULL);
    
    /* Wait for data or if buffer (resp. 1) is empty (resp. Command failed)*/
    while(TRUE){
	
	/* Stop the function when while is looping for 1 second to stop polling*/
	if (chTimeNow() - time > MS2ST(1000)){
	    sendStreamCommand(stream_close, stream , NULL , NULL);
	    writeSerial( "Timeout : buffer is empty\r\n");
	    break;
	}
	
	if(NULL != strstr(stream_buffer, command_failed) || NULL != strstr(stream_buffer, "1"))
	    break;
	else 
	    sendStreamCommand(stream_poll, stream , NULL , NULL);
    }
}

/* Function that sends hhtp_request and save th page in file */
static void saveWebPage( char * address , char * file){
    chMtxLock(&wifiAccessMtx);    
    
    /* Build http request command */
    strcat(msgWifi , http_get);
    strcat(msgWifi , address);
    strcat(msgWifi , endLine);
    
    save = TRUE;
    print = FALSE;

    /* Send http_request */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';
    
    int stream = atoi(stream_buffer);
   
    /* Create file and open it with writing mode */ 
    res = f_open(&fil,file,FA_WRITE | FA_CREATE_NEW);
    if (res == FR_EXIST){
	writeSerial("This file already exist\r\n");
	sendStreamCommand(stream_close, &stream , NULL , NULL);
    }
    else if (res){ 
        writeSerial("Cannot create this file\r\n");
	sendStreamCommand(stream_close, &stream , NULL , NULL);
    }
    else {	
	/* Read the first stream if available */
	polling(&stream);
	sendStreamCommand(stream_read, &stream , &dataRead , NULL);

	/* Read until stream is not closed */
	while (NULL == strstr(stream_buffer, command_failed)){
	    f_write(&fil,stream_buffer,dataSize-2,(void*)NULL);
	    polling(&stream);    
	    sendStreamCommand(stream_read, &stream , &dataRead , NULL);
	}
	save = FALSE;
	print =TRUE;
	writeSerial("Webpage saved\r\n");
    }
    f_close(&fil);
    chMtxUnlock();
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
void postAndRead( char * address , char * data){
    chMtxLock(&wifiAccessMtx);

    /* Build hhtp post request */
    strcat(msgWifi ,http_post);
    strcat(msgWifi , address);
    strcat(msgWifi , "?");
    strcat(msgWifi, data);
    strcat(msgWifi , urlencoded);

    /* Read the first stream */
    print = FALSE;
    save = TRUE;

    /* Send http_post */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';

    int stream = atoi(stream_buffer);
 
    /* Read the first stream if available */
    polling(&stream);
    sendStreamCommand(stream_read, &stream , &dataRead , NULL);

    /* Read until stream is not closed */
    while (NULL == strstr(stream_buffer, command_failed)){
	writeSerial(stream_buffer);
	polling(&stream);
   	sendStreamCommand(stream_read, &stream , &dataRead , NULL);
    }
    
    save = FALSE;
    print = TRUE;
    writeSerial("Response received\r\n");
    chMtxUnlock();
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

void uploadFile( char *address , char * localFile , char * remoteFile){
    chMtxLock(&wifiAccessMtx);
    /* Open file in reading mode */
    res = f_open(&fil,localFile,FA_OPEN_EXISTING | FA_READ);
    if (res) {
        writeSerial("Cannot read this file %d\r\n",res);
        f_close(&fil);
	chMtxUnlock();
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
   
    print = FALSE;
    save = TRUE;

    /* Send command to create a file */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';

    int stream = atoi(stream_buffer);
    
    int intBr;
    /* Read file in SD Card and send data to wifi module */ 
    while(TRUE){
	
	/* Fille buffer with file's data */
	res = f_read(&fil, writeBuff, DATA_WRITE ,&br );
	intBr = br;
	if(res){
	    writeSerial("Error when reading file %d", res);
	    break;
	}
	sendStreamCommand(stream_write, &stream , &intBr , writeBuff);
	if(NULL == strstr(stream_buffer, "Success")){
	    writeSerial("A stream not sent\r\n");
	    break;
	} 	
	if(br != DATA_WRITE){	    
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
    chMtxUnlock();
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

    print = FALSE;
    save = TRUE;

    /* Send wifi command to get page */
    wifiWriteByUsart(msgWifi, strlen(msgWifi));
    msgWifi[0] ='\0';
    
    int stream = atoi(stream_buffer);
    
    /* Read the first stream if available */
    polling(&stream);
    sendStreamCommand(stream_read, &stream , &dataRead , NULL);

    /* Read until stream is not closed */
    while (NULL == strstr(stream_buffer, command_failed)){
	/* Send each character */
	for (int i = 0 ; i < dataSize ; i++){
	    parseXML(stream_buffer[i]);
	}
	polling(&stream);
	sendStreamCommand(stream_read, &stream , &dataRead , NULL);
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


