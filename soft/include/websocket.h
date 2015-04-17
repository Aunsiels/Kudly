/**
 * \file websocket.h
 * \brief Websocket management functions
 * \author KudlyProject
 * \version 0.1
 * \date 04/16/2015
 *
 * Manages websockets connection, sending and receiving data
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "hal.h"
#include "ch.h"

/* Main receiving mailbox */
extern Mailbox * mb;

/* Stream buffer */
extern char stream_buffer[];
extern int dataSize;

/**
 * \brief Initializes a websocket connection
 */
void websocketInit(void);

/**
 * \brief Encodes a string a send it in a websocket packet
 * \param str The string to encode
 */
void websocketEncode(char * str);

/**
 * \brief Open a websocket to kudly.herokuapp.com:80/echo
 * \param chp    can be NULL
 * \param argc   can be 0
 * \param argv   can be NULL
 *
 * Can be launched in the shell with "wsinit"
 */
void cmdWebSocInit(BaseSequentialStream* chp, int argc, char * argv[]);

/**
 * \brief Unlocks both sending & receiving threads
 * \param chp    can be NULL
 * \param argc   can be 0
 * \param argv   can be NULL
 *
 * Can be launched in the shell with "ws"
 */
void cmdWebSoc(BaseSequentialStream * chp, int argc, char * argv[]);

/**
 * \brief Init function for websockets thread
 *
 * Need to be called before anything
 */
void streamInit(void);

/**
 * \brief send buffer to wifi module
 */
void sendToWS(void);

/**
 * \brief print data when streaming is enabled in wifi_manager.c
 */
void parseWebSocket(msg_t c);

/**
 * \brief Unlocks receiving thread
 * \param chp    can be NULL
 * \param argc   can be 0
 * \param argv   can be NULL
 *
 * Can be launched in the shell with "ws"
 */
void cmdDlWave(BaseSequentialStream * chp, int argc, char * argv[]);

/**
 * \brief Stop receiving thread
 * \param chp    can be NULL
 * \param argc   can be 0
 * \param argv   can be NULL
 *
 * Can be launched in the shell with "wsstoprecv"
 */
void cmdStopSend(BaseSequentialStream * chp, int argc, char * argv[]);

/**
 * \brief Stop sending thread
 * \param chp    can be NULL
 * \param argc   can be 0
 * \param argv   can be NULL
 *
 * Can be launched in the shell with "wsstopsend"
 */
void cmdStopRecv(BaseSequentialStream * chp, int argc, char * argv[]);

#endif
