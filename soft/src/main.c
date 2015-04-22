#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"
#include "sccb.h"
#include "wifi_manager.h"
#include "hug_sensors.h"
#include "hand_sensors.h"
#include "codec.h"
#include "camera.h"
#include "wifi_manager.h"
#include "imu.h"
#include "temperature.h"
#include "i2c_perso.h"
#include "ext_init.h"
#include "pir.h"
#include "application.h"

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    /* Initialize shell */
    shellPersoInit();

    /* Initialize SD card */
    sdPersoInit();

    /* Led initialization */
    ledInit();

    /* Init sccb */
    sccbInit();

    /* DCMI init */
    cameraInit();

    /* Read wifi by usart */
    usartRead();

    /* Initialize wifi */
    wifiInitByUsart();

    /* Init ADC hug sensors */
    initHugSensors();

    /* Init ADC hand sensors */
    initHandSensors();

    /* Init codec */
    codecInit();

    /* Init i2c bus */
    i2cPersoInit();
    
    /* IMU init */
    imuInit();

    /* Init temperature sensor */
    temperatureInit();

    /* Initialize Ext */
    extPersoInit();

    /* Pir initialization */
    pirInit();
    
    /* Initializes the application */
    // applicationInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
