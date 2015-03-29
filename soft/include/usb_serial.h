/**
 * \file usb_serial.h
 * \brief Serial over usb connexion
 * \author KudlyProject
 * \version 0.1
 * \date 29/03/2015
 *
 * Manages the serial over usb connection.
 *
 */

#ifndef USB_SERIAL_H
#define USB_SERIAL_H

/**
 *
 * \brief Initializes the usb over serial communication
 *
 */
void init_usb_serial (void);

/**
 *
 * \brief Writes datas over the usb.
 * \param buffer A buffer containing the datas to send.
 * \param length The length of the buffer.
 *
 * This function writes datas over the usb connection.
 *
 */

void write_serial(uint8_t * buffer, int size);


/**
 *
 * \brief Reads datas over the usb.
 * \param buffer A buffer where the received datas will be written.
 * \param length The length of the buffer.
 *
 * This function reads datas reveived on the usb.
 *
 */

void read_serial(uint8_t * buffer, int size);

#endif
