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

/* Mailing box for data received by wifi */
extern Mailbox mbReceiveWifi;

/* Signals a streaming */
extern volatile int streaming;

/* Mutex for wifi connexion */
extern Mutex wifiMtx;

/**
 *
 * \brief Send data by wifi using usart3
 * \param message The message to be sent
 * \param length The length of this message
 * \return 0 if timeout
 * Bloking function
 */
int wifiWriteByUsart(char * message, int length);

/**
 *
 * \brief Asynchronous writing function.
 * \param message The message to be sent
 * \param length The length of this message
 */
void wifiWriteNoWait(char * message, int length);
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

/**
 *
 * \brief Command to sleep wifi module
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 */
void cmdWifiSleep(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 *
 * \brief Command to wake up wifi module
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 */
void cmdWifiWakeUp(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Initialize the wifi again. Used for streaming.
 */
void wifiInitAgain(void);
#endif
