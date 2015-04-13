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
 * \brief Send data by wifi using usart3
 * \param message The message to be sent
 * \param length The length of this message
 * Bloking function
 */
void wifiWriteByUsart(char * message, int length);
void wifiWriteUnsigned(uint8_t * message, int length);


/**
 *
 * \brief Send command to wifi module
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 */
void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 *
 * \brief Initializes the wifi communication
 *
 * This function connects the board to the a406 network  
 */
void wifiInitByUsart(void);

#endif
