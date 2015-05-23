/**
 * \file dcmi.h
 * \brief DCMI interface
 */

#ifndef DCMI_H
#define DCMI_H

/**
 * \brief Initializes the camera
 */

void cameraInit(void);

/**
 * \brief Command to take a picture from the shell and write it in a file
 * \param chp The channel where the strings are written.
 * \param argc The number of arguments
 * \param argv The arguments
 */
void cmdCamera(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Set the brightness of the camera
 * \param brightness The brightness configuration. Possible values are :
 *     0x40 for Brightness +2,
 *     0x30 for Brightness +1,
 *     0x20 for Brightness 0,
 *     0x10 for Brightness -1,
 *     0x00 for Brightness -2
 */
void cameraSetBrightness(uint8_t brightness);

/**
 * \brief Configures the black and white mode.
 * \param blackWhite The black and white configuration. Possible values are :
 *     0x18 for B&W,
 *     0x40 for Negative,
 *     0x58 for B&W negative,
 *     0x00 for Normal
 */
void cameraSetBW(uint8_t blackWhite);

/**
 * \brief Sets the color effect
 * \param value1 The first value (see below)
 * \param value2 The second value (see below)
 *
 * Possible values are :
 *     value1 = 0x40, value2 = 0xa6 for Antique,
 *     value1 = 0xa0, value2 = 0x40 for Bluish,
 *     value1 = 0x40, value2 = 0x40 for Greenish,
 *     value1 = 0x40, value2 = 0xc0 for Reddish
 */
void cameraSetColorEffect(uint8_t value1, uint8_t value2);

/**
 * \brief Sets the contrast value.
 * \param value1 The first value (see below)
 * \param value2 The second value (see below)
 *
 * Possible values are :
 *     value1 = 0x28, value2 = 0x0c for Contrast +2,
 *     value1 = 0x24, value2 = 0x16 for Contrast +1,
 *     value1 = 0x20, value2 = 0x20 for Contrast 0,
 *     value1 = 0x1c, value2 = 0x2a for Contrast -1,
 *     value1 = 0x18, value2 = 0x34 for Contrast -2,
 */
void cameraSetContrast(uint8_t value1, uint8_t value2);

/**
 * \brief Takes a photo
 * \param photoName The name of the photo
 */
void photo(char * photoName);

#endif
