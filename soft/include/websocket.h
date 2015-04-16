#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "hal.h"
#include "ch.h"

/* Main receiving mailbox */
extern Mailbox * mb;

//Boolean values, both defined in wifi_manager.c
extern bool_t streaming;
extern bool_t print;
extern char stream_buffer[];

/**
 * \brief Initializes a websocket connection
 */
void websocketInit(void);

/**
 * \brief Encodes a string a send it in a websocket packet
 * \param str The string to encode
 */
void websocketEncode(char * str);
void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]);
void cmdWebSoc(BaseSequentialStream * chp, int argc, char * argv[]);
void streamLaunch(BaseSequentialStream * chp, int argc, char * argv[]);
void streamInit(void);
void sendToWS(char * str);

void parseWebSocketBuffer(void);
void parseWebSocket(msg_t c);

void cmdDlWave(BaseSequentialStream * chp, int argc, char * argv[]);

#endif
