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

extern Mailbox mbCodecOut;

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
 * \brief Software reset of the codec 
 *
 * This function start the SPI bus, configures SCI_MODE, SCI_CLOCKF, SCI_AUDATA and SCI_VOL registers  
 */

void codecReset(void);

/**
 *
 * \brief Set the volume (input and output)
 *
 * This function set the volume (can't be used during playbak or encoding)   
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

/**
 *
 * \brief Shell command for encoding and decoding sounds at the same time
 *
 * This function enables the Codec (Fullduplex) function in a shell command 
 * It is used for streaming audio : the data are stocked in a mailbox, and readen from a mailbox
 */

void cmdFullDuplex(BaseSequentialStream *, int, char *[]);

/**
 *
 * \brief Shell command for stop encoding a sound
 *
 * This function stop the encoding of a sound  
 */

void cmdStop(BaseSequentialStream *, int, char *[]);

/**
 *
 * \brief Shell command for volume control 
 *
 * This function control volume (may be set from 0 to 10)  
 */

void cmdVolume(BaseSequentialStream *, int, char *[]);

/**
 *
 * \brief Shell command for control of the sound during playback
 *
 * This function control volume and allow interrupt playback in a shell command  
 */

void cmdControl(BaseSequentialStream *, int, char *[]);

/**
 * 
 * \brief Shell command for testing the micro
 *
 * Takes a duration and change the intenity of a led
 */
void cmdTestVolume(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* CODEC_H */
