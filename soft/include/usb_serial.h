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
void initUsbSerial (void);

/**
 *
 * \brief Writes datas over the usb.
 * \param fmt Formatting string
 *
 * \details This function implements a minimal @p printf() like functionality
 *     with output on a @p BaseSequentialStream.
 *     The general parameters format is:
 *         [-][width|*][.precision|*][l|L]p.
 *     The following parameter types (p) are supported:
 *          - <b>x</b> hexadecimal integer.
 *          - <b>X</b> hexadecimal long.
 *          - <b>o</b> octal integer.
 *          - <b>O</b> octal long.
 *          - <b>d</b> decimal signed integer.
 *          - <b>D</b> decimal signed long.
 *          - <b>u</b> decimal unsigned integer.
 *          - <b>U</b> decimal unsigned long.
 *          - <b>c</b> character.
 *          - <b>s</b> string.
 *          .
 *
 */

void writeSerial(const char * fmt,...);


/**
 *
 * \brief Reads datas over the usb.
 * \param buffer A buffer where the received datas will be written.
 * \param length The length of the buffer.
 *
 * This is a blocking function that reads datas reveived on the usb.
 *
 */

void readSerial(uint8_t * buffer, int size);

/*
 * \def SerialUSBDriver SDU1
 * \brief The driver for the serial over usb
 */
extern SerialUSBDriver SDU1;

#endif
