#include "ch.h"
#include "hal.h"
#include "temperature.h"
#include "usb_serial.h"

/* Errors flags for I2C communication */
static i2cflags_t errors1 = 0;
static i2cflags_t errors2 = 0;

/* Adress of the temperature sensor of the I2C module */
#define temperatureAddr 0b0011000

/* Adress of the registers on the temperature sensor */
#define CONFIG 0x01
#define AMB_TEMP 0x05
#define RESOLUTION 0x08

/* I2C configuration (400kHz is the fastest speed of the temperature sensor)*/
static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

/* This function reads in a register of the sensor ( the parameter is the address of the register ) */
uint16_t readRegisterT(uint8_t addr){
    uint16_t data;
    uint8_t * tabData = (uint8_t *)&data;
    static msg_t status1 = RDY_OK;
    static msg_t status2 = RDY_OK;
    /* We need to write in the register pointer the address of the register we want to read */
    i2cAcquireBus(&I2CD2);
    status1 = i2cMasterTransmit(&I2CD2, temperatureAddr, &addr, 1, NULL, 0);
    /* And then we read the contain of the register */
    status2 = i2cMasterReceive(&I2CD2, temperatureAddr, tabData, 2);
    i2cReleaseBus(&I2CD2);
    /* Test if the transmit succeeded */
    if (status1 != RDY_OK){
        errors1 = i2cGetErrors(&I2CD2);
        writeSerial("Error : %u during the transmit\r\n\r\n",errors1);
        i2cStop(&I2CD2);
        temperatureInit();
    }
    /* Test if the receive succeded */
    if (status2 != RDY_OK){
        errors2 = i2cGetErrors(&I2CD2);
        writeSerial("Error : %u during the receive\r\n\r\n",errors2);
        i2cStop(&I2CD2);
        temperatureInit();
    }
    /* We reveive the MSB first so we need to exchange the MSB and the LSB before returning data */
    uint8_t temp = tabData[0];
    tabData[0] = tabData[1];
    tabData[1] = temp;

    return data;
}

/* This function writes in a register of the sensor */
void writeRegisterT(uint8_t addr, uint16_t data, int size){
    static msg_t status = RDY_OK;
    uint8_t * ptrLSB = (uint8_t *)&data;
    uint8_t * ptrMSB = ptrLSB+1;
    uint8_t command[3];
    /* For writing a register, we need to send the register address (command[0]), the data MSB (command[1]) and the data LSB (command[2]) */
    command[2] = ptrLSB[0];
    command[1] = ptrMSB[0];
    command[0] = addr;
    /* We need to write in the register pointer the address of the register we want to write in, and then we send the contain */
    i2cAcquireBus(&I2CD2);
    status = i2cMasterTransmit(&I2CD2, temperatureAddr,command, size, NULL, 0);
    i2cReleaseBus(&I2CD2);
    /* Test if the transmit succeeded */
    if (status != RDY_OK){
        errors1 = i2cGetErrors(&I2CD2);
        writeSerial("Error : %u\r\n",errors1);
    }
}


void temperatureInit(void){
    /* Set the I2C pin in I2C mode */
    palSetPadMode(GPIOB, GPIOB_I2C_SCL, PAL_MODE_ALTERNATE(4)| PAL_STM32_OTYPE_OPENDRAIN);
    palSetPadMode(GPIOB, GPIOB_I2C_SDA, PAL_MODE_ALTERNATE(4)| PAL_STM32_OTYPE_OPENDRAIN);

    i2cStart(&I2CD2, &i2cfg);
    /* Set the power up default mode */
    writeRegisterT(CONFIG,0,3);
    /* Choose a resolution of 0.5°C */
    writeRegisterT(RESOLUTION,0x0,2);
}

uint16_t getTemperatureHandled(void){
    uint16_t content=0;
    uint8_t * ptrLSB = (uint8_t *)&content;
    uint8_t * ptrMSB = ptrLSB+1;
    uint16_t temp=0;
    uint8_t upperByte;
    uint8_t lowerByte;

    /* Reads the temperature value in the register AMBIENT TEMPERATURE */
    content=readRegisterT(AMB_TEMP);
    upperByte=ptrMSB[0];
    lowerByte=ptrLSB[0];

    /* Clear the alert flags */
    upperByte=upperByte & 0x1f;
    /* Test if the sign bit is 1 or 0 (bit 12) and then calculate the temperature */
    if((upperByte & 0x10) == 0x10){
        upperByte = upperByte & 0x0f;
        temp = 256 - (((upperByte<<4)&0xf0) + ((lowerByte>>4)&0x0f));
    }else
        temp = (((upperByte<<4)&0xf0) + ((lowerByte>>4)&0x0f));

    return temp;
}

uint16_t getTemperatureNotHandled(void){
    uint16_t content=0;

    /* Reads the temperature value in the register */
    content=readRegisterT(AMB_TEMP);

    /* Returns the value without any processing */
    return content;
}
void cmdTemperature(BaseSequentialStream *chp, int argc, char *argv[]){
    (void) argv;
    (void) argc;
    (void) chp;
    uint16_t temperatureHandled, temperatureNotHandled;

    /* Gets back the temperature value processed */
    temperatureHandled=getTemperatureHandled();
    writeSerial("Temperature : %d\r\n",temperatureHandled);
    /* Gets back the temperature value not processed */
    temperatureNotHandled=getTemperatureNotHandled();
    writeSerial("Temperature Handled : %d\r\n", temperatureNotHandled);
    chThdSleepMilliseconds(50);
}
