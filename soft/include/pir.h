/**
 * \file pir.h
 * \brief This package initializes the pir.
 * The PIR uses EXTI so, extPersoInit in ext_init must be call before starting the pir.
 */

#ifndef LED_H
#define LED_H

/**
 * \brief This function initializes the pir sensor.
 */
void pirInit(void);

#endif
