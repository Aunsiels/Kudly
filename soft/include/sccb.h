/**
 * \file sccb.h
 * \brief A sccb driver for the camera.
 */

#ifndef SCCB_H
#define SCCB_H

/**
 * \brief Initializes the sccb for the camera
 */
void sccbInit(void);

/**
 * \brief Write data to a register
 * \param registerAddress The address of register where the data are written
 * \param value The new value of this regiter
 * \return 1 if the transmission succeeded, else 0
 */
int sccbWrite(uint8_t registerAddress, uint8_t value);

#endif
