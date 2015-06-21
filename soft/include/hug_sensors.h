/**
 * \file hug_sensors.h
 * \brief Control ADC hug sensors
 * \author KudlyProject
 * \version 0.1
 * \date 08/04/2015
 *
 * Manages the wifi connection.
 *
 */

#ifndef _HUG_SENSORS_H_
#define _HUG_SENSORS_H_

/**
 *
 * \brief Initializes ADC hug sensors
 *
 * This function configures inputs and ouput pins for ADC conversion
 */
void initHugSensors(void);

/**
 *
 * \brief Launches ADC conversion
 *
 * Starts a conversion and print on shell values
 */
void cmdHugSensors(BaseSequentialStream *chp, int argc, char *argv[]);

/**
 * \brief Gives the values of the hugs sensors
 * \return The values of the hug sensors, with on on the lowest part and the
 * other one on the highest part
 */
uint32_t getHugValues (void);

#endif
