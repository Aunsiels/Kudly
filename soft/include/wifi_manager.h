/**
 * \file wifi_manager.h
 * \brief Read all usart communication
 * \author KudlyProject
 * \version 0.1
 * \date 09/04/2015
 *
 * Manages usart reception
 *
 */
#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

/* Stream buffer */
extern char stream_buffer[];

/**
 *
 * \brief Envent source when data is received
 *
 */
extern EventSource srcEndToReadUsart;

/**
 *
 * \brief Launches thread to read data, and parses responses
 *
 * Parses header and data on usart3 channel
 */
void usartRead(void);

/**
 *
 * \brief Save web page in file
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * Send http request, read the web page and saves it on specified file
 */
void cmdWifiGet(BaseSequentialStream *chp, int argc, char * argv[]);

/**
 *
 * \brief Save web page in file
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * Send http post, send data and wait for response
 */
void cmdWifiPost(BaseSequentialStream *chp, int argc, char * argv[]);

/**
 *
 * \brief Upload file on server
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * Load file in wifi module flash, upload it on server and delete 
 * in wifi module flash
 */
void cmdWifiUpload(BaseSequentialStream *chp, int argc, char * argv[]);

/**
 *
 * \brief Execute command on server
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * Read a web page and execute the command written on it
 */
void cmdWifiXml(BaseSequentialStream *chp, int argc, char * argv[]);

#endif
