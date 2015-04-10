
/**
 * \file wifi.h
 * \brief Wifi feature use usart3
 * \author KudlyProject
 * \version 0.1
 * \date 07/04/2015
 *
 * Manages the wifi connection.
 *
 */

#ifndef _WIFI_H_
#define _WIFI_H_

extern Mailbox mbReceiveWifi;
void externBroadcast(void);
/**
 *
 * \brief Initializes the wifi communication
 *
 * This function connects the board to the a406 network  
 */
void wifiInitByUsart(void);

/**
 *
 * \brief Send data by wifi using usart3
 *
 */
void wifiWriteByUsart(char * message, int length);

/**
 *
 * \brief Stops the wifi communication
 *
 */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 *
 * \brief Launches thread to interpret command from wifi
 *
 */
void wifiCommands(void);

/**
 *
 * \breif TCP connection
 *
 */
void wifiStartStreaming(char * socketUrl, int port);

#endif
