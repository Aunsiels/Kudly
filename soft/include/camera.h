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

#endif
