/**
 * \file sccb.h
 * \brief A sccb driver for the camera.
 * \author KudlyProject
 */

#ifndef SCCB_H
#define SCCB_H

/**
 * \brief Initializes the sccb for the camera
 */
void sccbInit(void);

/**
 * \brief Write data to a register of the camera
 * \param registerAddress The address of register where the data are written
 * \param value The new value of this regiter
 * \return 1 if the transmission succeeded, else 0
 */
int sccbWrite(uint8_t registerAddress, uint8_t value);

/**
 * \brief Read the values in a register of the camera.
 * \param[in] registerAddress The address of the register to read
 * \param[out] value The value read
 * \return 1 if the transmission succeeded, else 0
 */
int sccbRead(uint8_t registerAddress, uint8_t * value);

/**
 * \brief Command the write in a register from the shell
 * \param chp The channel where the strings are written.
 * \param argc The number of arguments
 * \param argv The arguments
 */
void cmdWrite(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Command the read in a register from the shell
 * \param chp The channel where the strings are written.
 * \param argc The number of arguments
 * \param argv The arguments
 */
void cmdRead(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
