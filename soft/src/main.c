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

int main(void) {

    halInit();
    chSysInit();

    /* Clear pad to break wifi factory reset */
    palClearPad(GPIOB, GPIOB_SPI2_MISO);
    chThdSleepMilliseconds(100);

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

    while(1) {
        uint32_t readValues;
        uint16_t * low = (uint16_t *) &readValues;
        uint16_t * high = low + 1;
        /* Begins by hands */
        readValues = getHandValues();
        /* Separate low and high */
        if (*low > 500){
            cmdLedtest((BaseSequentialStream *) &SDU1, 0, NULL);
        }
        if (*high > 500){
            
        }
    }

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
