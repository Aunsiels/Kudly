#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "hal.h"
#include "ch.h"

/**
 * \brief Initializes a websocket connection
 */
void websocketInit(void);

/**
 * \brief Encodes a string a send it in a websocket packet
 * \param str The string to encode
 */
void websocketEncode(char * str);
void cmdWebSoc(BaseSequentialStream * chp, int argc, char * argv[]);

#endif
