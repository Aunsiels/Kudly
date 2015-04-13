#include "ch.h"
#include "hal.h"
#include "imu.h"
#include "usb_serial.h"

static i2cflags_t errors = 0;

#define imuAddr 0b1101000

/* I2C configuration (400kHz is the fastest speed of the imu)*/
static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

uint8_t readRegister(uint8_t addr){
    static uint8_t data;
    static msg_t status = RDY_OK;
    /* Gain access of I2C bus, and collect the data of the register */
    i2cAcquireBus(&I2CD2);
    status = i2cMasterTransmit(&I2CD2, imuAddr, &addr, 1, &data, 8);
    i2cReleaseBus(&I2CD2);
    if (status != RDY_OK){
	errors = i2cGetErrors(&I2CD2);
	writeSerial("Error : %u\r\n\r\n",errors);
	i2cStop(&I2CD2);
	imuInit();
    }
    
    return data;
}

int16_t readRegister16(uint8_t addr){
    (void) addr;

    int16_t data;
    uint8_t * lsb = (uint8_t *)&data;
    uint8_t * msb = lsb + 1;
    
    *msb = readRegister(addr);
    addr++;
    *lsb = readRegister(addr);

    return data;
}

void writeRegister(uint8_t addr, uint8_t data){
    static msg_t status = RDY_OK;
    (void) data;
    uint8_t command[2];
    command[1] = data;
    command[0] = addr;
    /* Gain access of I2C bus, and write the data to the register */
    i2cAcquireBus(&I2CD2);
    status = i2cMasterTransmit(&I2CD2, imuAddr,command, 2, NULL, 0);
    i2cReleaseBus(&I2CD2);
    if (status != RDY_OK){
	errors = i2cGetErrors(&I2CD2);
	writeSerial("Error : %u\r\n",errors);
	i2cStop(&I2CD2);
	imuInit();
    }
   
}


void imuInit(void){
    /* Set the I2C pin in I2C mode */
    palSetPadMode(GPIOB, GPIOB_I2C_SCL, PAL_MODE_ALTERNATE(4)| PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOB, GPIOB_I2C_SDA, PAL_MODE_ALTERNATE(4)| PAL_STM32_OSPEED_HIGHEST);
   
    i2cStart(&I2CD2, &i2cfg);
    writeRegister(0x6b,0);
}

void cmdImu(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    (void) argc;
    (void) chp;
    
    while(1){
	writeSerial("Accel X : %d\r\n",readRegister16(0x3b));
	chThdSleepMilliseconds(50);
    }
}



