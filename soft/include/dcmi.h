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

#endif
