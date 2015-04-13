#ifndef IMU_H
#define IMU_H

void imuInit(void);
void cmdImu(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* IMU_H */
