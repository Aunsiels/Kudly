#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "usb_serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE,
    RECEIVE_LOG
};


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
                                writeSerial("%d bytes : ", headerSize);
                                wifiReadState = RECEIVE_RESPONSE;
                            } else {
                                writeSerial("Log : ");
                                wifiReadState = RECEIVE_LOG;
                            }
                            break;
                    }

                    h++;
                    break;
                case RECEIVE_RESPONSE:
                    writeSerial("%c", c);

                    dataCpt++;
                    if(dataCpt == headerSize) {
                        // DO SOMETHING
                        wifiReadState = IDLE;
                    }
                    break;
                case RECEIVE_LOG:
                    writeSerial("%c", c);
                    
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

/*
 * Streaming through socket
 */
void wifiStartStreaming(char * socketUrl, int port) {
    (void)socketUrl;
    (void)port;

    writeSerial("Starting streaming...\n\r");

    static char tcpConnect[] = "tcp_client 137.194.43.54 6789";
    static char wifiExit[] = "wifi";

    static char testHello[] = "Salut ! Ça va bien ?\r\n";
    //static char testString[] = "Ici le module wifi wesh\r\n";

    // setting up TCP connection
    wifiWriteByUsart(tcpConnect, sizeof(tcpConnect));
    wifiWriteByUsart(wifiExit, sizeof(wifiExit));

    wifiWriteByUsart(testHello, sizeof(testHello));

    chThdSleep(TIME_INFINITE);
}
