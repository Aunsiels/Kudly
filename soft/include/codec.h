/**
 * \file codec.h
 * \brief Control audio codec
 * \author KudlyProject
 * \version 0.1
 * \date 09/04/2015
 *
 * Manages the ADC hand sensors conversions.
 *
 */

#ifndef CODEC_H
#define CODEC_H

/**
 *
 * \brief Initializes audio codec
 *
 * This function configures pad modes to enable SPI communication  
 */

void codecInit(void);

/**
 *
 * \brief Initializes codec registers 
 *
 * This function configures SCI_CLOCKF, SCI_AUDATA and SCI_VOL registers in order to consume less energy   
 */

void codecLowPower(void);

/**
 *
 * \brief Writes 16 bits data in the codec RAM
 *
 * This function writes in the registers SCI_WRAMADDR and SCI_WRAM in order to send 16 bits data   
 */

void writeRam(uint16_t,uint16_t);

/**
 *
 * \brief Writes 32 bits data in the codec RAM
 *
 * This function writes once in the register SCI_WRAMADDR and twice in the register SCI_WRAM in order to send 32 bits data   
 */

void writeRam32(uint16_t,uint32_t);

/**
 *
 * \brief Reads 16 bits data in the codec RAM
 *
 * This function writes in the register SCI_WRAMADDR, the address of our data and then returns the contain of the register on 16 bits   
 */

uint16_t readRam(uint16_t);

/**
 *
 * \brief Reads 32 bits data in the codec RAM
 *
 * This function writes in the register SCI_WRAMADDR, the address and the address +1 of our data and then returns the contain of the registers on 32 bits  
 */

uint32_t readRam32(uint16_t);

/**
 *
 * \brief Changes the output volume
 *
 * This function writes in the register SCI_VOL a value of a sound volume. Its argument is an int from 0 to 10, 0 for min and 10 for max  
 */

void codecVolume(int);

/**
 *
 * \brief Displays a music file
 *
 * This function follows the sending protocol wrote in the VS1063 datasheet. It is also able to send a file from the SDcard 
 */

void cmdPlay(BaseSequentialStream *, int, char *[]);

/**
 *
 * \brief Shell command for encoding a sound
 *
 * This function enables the Encode function in a shell command  
 */

void cmdEncode(BaseSequentialStream *, int, char *[]);
void cmdControl(BaseSequentialStream *, int, char *[]);

#endif /* CODEC_H */
