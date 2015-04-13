/**
 * \file temperature.h
 * \brief Control temperature sensor
 * \author KudlyProject
 * \version 0.1
 * \date 17/04/2015
 *
 * Manages the temperature sensor conversions.
 *
 */

#ifndef TEMP_H
#define TEMP_H

/**
 * \brief Initializes the temperature sensor
 */

void temperatureInit(void);

/**
 * \brief Function which captures the temperature and handle it to display in °c
 */

uint16_t getTemperatureHandled(void);

/**
 * \brief Function which captures the temperature not handled ( The register's value )
 */

uint16_t getTemperatureNotHandled(void);

/**
 * \brief This function launches the temperature capture ( no arguments )
 */

void cmdTemperature(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* TEMP_H */
