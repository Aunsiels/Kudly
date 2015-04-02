#ifndef SD_PERSO_H
#define SD_PERSO_H

#include "hal.h"
#include "ch.h"
#include "ff.h"

/* Command tree */
void cmdTree(BaseSequentialStream *chp, int argc, char *argv[]);

/* ls command */
void cmdLs(BaseSequentialStream *chp, int argc, char *argv[]);

/* pwd command */
void cmdPwd(BaseSequentialStream *chp, int argc, char *argv[]);

/* cd command */
void cmdCd(BaseSequentialStream *chp, int argc, char *argv[]);

/* rm command */
void cmdRm(BaseSequentialStream *chp, int argc, char *argv[]);

/* Mv command */
void cmdMv(BaseSequentialStream *chp, int argc, char *argv[]);

/* cat command */
void cmdCat(BaseSequentialStream *chp, int argc, char *argv[]);

/* mkdir command */
void cmdMkdir(BaseSequentialStream *chp, int argc, char *argv[]);

/* touch command */
void cmdTouch(BaseSequentialStream *chp, int argc, char *argv[]);

/* Initializes the sd card spi */
void sdInit(void);

/* test SD functionalities */
void testSd(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
