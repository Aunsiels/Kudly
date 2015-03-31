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

/* 
 * \brief Command tree for the shell
 * \param chp The stream where the string will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdTree(BaseSequentialStream *chp, int argc, char *argv[]);

/* 
 * \brief ls command for the shell
 * \param chp The stream where the string will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdLs(BaseSequentialStream *chp, int argc, char *argv[]);

/* \brief pwd command for the shell
 * \param chp The stream where the string will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdPwd(BaseSequentialStream *chp, int argc, char *argv[]);

/* \brief cd command for the shell
 * \param chp The stream where the string will be written.
 * \param argc The number of arguments
 * \param argv The parameters
 * */
void cmdCd(BaseSequentialStream *chp, int argc, char *argv[]);

/* 
 * \brief Initializes the sd card spi 
 * */
void sdPersoInit(void);

#endif
