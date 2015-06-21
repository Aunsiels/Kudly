/**
 * \file sd_perso.h
 * \brief Sd manager
 * \author KudlyProject
 * \version 0.1
 * \date 30/03/2015
 *
 * This file manages the sd card.
 *
 */

#ifndef SD_PERSO_H
#define SD_PERSO_H

#include "hal.h"
#include "ch.h"
#include "ff.h"

/**
 * \brief Is the sd card ready ?
 * \return The state of the sd card.
 */
bool_t sdIsReady (void);

/**
 * \brief Command tree for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdTree(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Command cat for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdCat(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief ls command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdLs(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief pwd command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdPwd(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief cd command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdCd(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief remove command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdRm(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief move command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdMv(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief make directory command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdMkdir(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief touch command for the shell
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdTouch(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Initializes the sd card spi
 * */
void sdPersoInit(void);

/**
 * \brief Test SD functionalities
 * \param chp The stream where the strings will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 *
 * This function tests some functions used on the SD such as writting in a file,
 * reading a file, create a directory...
 * */
void testSd(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Write data in a file
 * \param filename The name of the file
 * \param buf The buffer that contains data
 * \param length The number of data to write
 * \return The error code
 */
FRESULT writeFile(char * filename, char * buf, UINT length);

#endif
