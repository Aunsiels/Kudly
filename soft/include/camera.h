/**
 * \file dcmi.h
 * \brief DCMI interface
 */

#ifndef DCMI_H
#define DCMI_H

/**
 * \brief Initializes the DCMI
 */

void dcmiInit(void);

/**
 * \brief Start a conversion
 * \param buf Where the data will be store
 * \param nbrData The number of data to transfert
 */

void dcmiStartConversion(uint32_t * buf, int nbrData);


/**
 * \brief Command to take a picture from the shell and write it in a file
 * \param chp The channel where the strings are written.
 * \param argc The number of arguments
 * \param argv The arguments
 */
void cmdDcmi(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
