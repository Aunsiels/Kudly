/**
 * \file imu.h
 * \brief Control the imu
 * \author KudlyProject
 * \version 0.1
 * \date 17/04/2015
 *
 * Manages the imu conversions
 *
 */

#ifndef IMU_H
#define IMU_H

/**
 * \brief Initializes the imu
 */

void imuInit(void);

/**
 * \brief This function launches the IMU thread
 */

void cmdImu(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* IMU_H */
