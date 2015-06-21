/**
 * \file hand_sensors.h
 * \brief Control ADC hand sensors
 * \author KudlyProject
 * \version 0.1
 * \date 08/04/2015
 *
 * Manages the ADC hand sensors conversions.
 *
 */

#ifndef _HAND_SENSORS_H_
#define _HAND_SENSORS_H_

/**
 *
 * \brief Initializes ADC hand sensors
 *
 * This function configures inputs and ouput pins for ADC conversion
 */
void initHandSensors(void);

/**
 *
 * \brief Launches ADC conversion
 *
 * Starts a conversion and print on shell values
 */
void cmdHandSensors(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Get the values of the hands sensors
 *
 * \return The lsb are the right and the msb are the left sensors
 */
uint32_t getHandValues (void);

#endif
