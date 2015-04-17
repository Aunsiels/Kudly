/**
 * \file imu.h
 * \brief IMU interface
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
