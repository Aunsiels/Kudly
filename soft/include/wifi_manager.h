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

/**
 *
 * \brief Reads data over the usart.
 *
 * Parses header and data on usart3 channel
 */
void usartRead(void);

#endif
