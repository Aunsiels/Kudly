/**
 * \file pir.h
 * \brief This package initializes the pir.
 * The PIR uses EXTI so, extPersoInit in ext_init must be call before starting the pir.
 */

#ifndef PIR_H
#define PIR_H

/**
 * Event source to listen to if you want to see if there is a mouvement.
 */
//extern EventSource pirEvent;

/**
 * \brief This function initializes the pir sensor.
 */
void pirInit(void);

/*
 * \brief Tests if the pir works, to use in a terminal. Does 10 times a change.
 * \param chp The channel where the string are written
 * \param argc The number of params
 * \param argc The parameters
 */
void testPir(BaseSequentialStream *chp, int argc, char *argv[]);


#endif
