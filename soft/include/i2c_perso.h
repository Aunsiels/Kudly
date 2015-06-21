/**
 * \file i2c_perso.h
 * \brief Control i2c bus for imu and temperature sensor
 * \author KudlyProject
 * \version 0.1
 * \date 17/04/2015
 *
 */

#ifndef _I2C_PERSO_
#define _I2C_PERSO_

/**
 *
 * \brief Config i2c shared by imu and temperature sensor
 *
 */
extern const I2CConfig i2cfg;

/**
 *
 * \brief Init i2c bus for imu and temperature sensor
 *
 * Starts i2c
 */
void i2cPersoInit(void);

#endif
