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

void cmdWifiPost(BaseSequentialStream *chp, int argc, char * argv[]);

#endif