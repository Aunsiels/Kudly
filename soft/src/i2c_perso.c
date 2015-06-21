#include "ch.h"
#include "hal.h"
#include "i2c_perso.h"

/* I2C configuration (400kHz is the fastest speed of the imu)*/
const I2CConfig i2cfg = {
    OPMODE_I2C,
    200000,
    FAST_DUTY_CYCLE_2,
};

void i2cPersoInit(void) {
    /* Set the I2C pin in I2C mode and open drain */
    palSetPadMode(GPIOB, GPIOB_I2C_SCL, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    palSetPadMode(GPIOB, GPIOB_I2C_SDA, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

    /* Wait for the IMU to start */
    chThdSleepMilliseconds(100);

    i2cStart(&I2CD2, &i2cfg);
}
