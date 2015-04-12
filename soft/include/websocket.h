#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "hal.h"
#include "ch.h"

/* Main receiving mailbox */
extern Mailbox * mb;

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

void sendToWS(char * str);

#endif
