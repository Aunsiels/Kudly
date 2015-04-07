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

/**
 *
 * \brief Initializes the wifi communication
 *
 * This function connects the board to the a406 network  
 */
void wifiInitByUsart(void);

/**
 *
 * \brief Launches the wifi reading
 *
 * This function starts two threads to read data from wifi and parses data to ligth LEDs
 */
void wifiReadByUsart(void);

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
 * \brief Test function for wifi
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * This function reads the kudly website and lights LED with values written on it
 *
 */
void cmdWifiTest(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
