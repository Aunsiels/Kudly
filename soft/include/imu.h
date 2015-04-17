<<<<<<< HEAD
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

=======
#ifndef IMU_H
#define IMU_H

void imuInit(void);
>>>>>>> temperature
void cmdImu(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* IMU_H */
