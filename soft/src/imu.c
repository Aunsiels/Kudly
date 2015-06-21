#include "ch.h"
#include "hal.h"
#include "imu.h"
#include "usb_serial.h"
#include "led.h"
#include "i2c_perso.h"
#include "wifi_manager.h"

static i2cflags_t errors = 0;

#define ABS(x) (((x) < 0) ? -(x) : (x))

/* Adress of the imu on the I2C bus */
#define imuAddr 0b1101000

/* Adress of the registers on the imu */
#define PWR_MGMT_1 0x6b
#define PWR_MGMT_2 0x6c
#define ACCEL_X    0x3b
#define ACCEL_Y    0x3d
#define ACCEL_Z    0x3f
#define GYRO_X     0x43
#define GYRO_Y     0x45
#define GYRO_Z     0x47
#define TEMP       0x41

/* Configuration of power management registers */
#define SLEEP      (1 << 6)
#define CYCLE      (1 << 5)
#define TEMP_DIS   (1 << 4)
#define F_1_25     (0b00 << 6)
#define F_5        (0b01 << 6)
#define F_20       (2 << 6)
#define F_40       (3 << 6)
#define GYRO_DIS   0b111

static WORKING_AREA(waImu, 128);

uint8_t readRegister(uint8_t addr) {
    static uint8_t data;
    static msg_t status = RDY_OK;
    /* Gain access of I2C bus, and collect the data of the register */
    i2cAcquireBus(&I2CD2);
    status = i2cMasterTransmit(&I2CD2, imuAddr, &addr, 1, &data, 1);
    i2cReleaseBus(&I2CD2);
    if (status != RDY_OK) {
        errors = i2cGetErrors(&I2CD2);
        writeSerial("Error : %u\r\n",errors);
    }
    return data;
}

int16_t readRegister16(uint8_t addr) {
    (void) addr;

    int16_t data;
    uint8_t * lsb = (uint8_t *)&data;
    uint8_t * msb = lsb + 1;
    *msb = readRegister(addr);
    addr++;
    *lsb = readRegister(addr);

    return data;
}

msg_t writeRegister(uint8_t addr, uint8_t data) {
    static msg_t status = RDY_OK;

    uint8_t command[2];
    command[1] = data;
    command[0] = addr;
    /* Gain access of I2C bus, and write the data to the register */
    i2cAcquireBus(&I2CD2);
    status = i2cMasterTransmit(&I2CD2, imuAddr,command, 2, NULL, 0);
    i2cReleaseBus(&I2CD2);
    if (status != RDY_OK) {
        errors = i2cGetErrors(&I2CD2);
        writeSerial("Error : %u\r\n",errors);
        return status;
    }
    return status;
}

void imuInit(void) {
    /* Set Imu in low power mode */
    writeRegister(PWR_MGMT_1,CYCLE | TEMP_DIS);
    writeRegister(PWR_MGMT_2,F_1_25 | GYRO_DIS);
}


static msg_t threadImu(void *arg) {
    (void) arg;

    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    while(1) {
        accel_x = ABS(readRegister16(ACCEL_X));
        accel_y = ABS(readRegister16(ACCEL_Y));
        accel_z = ABS(readRegister16(ACCEL_Z));
        chThdSleepMilliseconds(1250);
        if(((accel_x - ABS(readRegister16(ACCEL_X))) > 1000) |
                ((accel_y - ABS(readRegister16(ACCEL_Y))) > 1000) |
                ((accel_z - ABS(readRegister16(ACCEL_Z))) > 1000))
            postAndRead("kudly.herokuapp.com/activity","value=1");
        else
            postAndRead("kudly.herokuapp.com/activity","value=0");
    }

    return(0);
}

void cmdImu(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void) argv;
    (void) argc;
    (void) chp;

    static int first = 1;

    if (first)
        chThdCreateStatic(waImu, sizeof(waImu),NORMALPRIO, threadImu,NULL);
    first = 0;
}
