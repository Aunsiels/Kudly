/**
 * \file pir.h
 * \brief This package initializes the pir.
 * The PIR uses EXTI so, extPersoInit in ext_init must be call before starting the pir.
 */

#ifndef LED_H
#define LED_H

/**
 * Event source to listen to if you want to see if there is a mouvement.
 */
EVENTSOURCE_DECL(pirEvent);

/**
 * \brief This function initializes the pir sensor.
 */
void pirInit(void);

#endif
