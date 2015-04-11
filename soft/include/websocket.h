#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "hal.h"
#include "ch.h"

/**
 * \brief Initializes a websocket connection
 */
void websocketInit(void);

void cmdWebSoc(BaseSequentialStream * chp, int argc, char * argv[]);

#endif
