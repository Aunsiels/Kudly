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
    
    /* Read wifi by usart */
    usartRead();

    /* DCMI init */
    cameraInit();

    /* Initialize wifi */
    wifiInitByUsart();

    /* Init ADC hug sensors */
    initHugSensors();

    /* Init ADC hand sensors */
    initHandSensors();

    ledInit();

    /* Init codec */
    codecInit();

    while(TRUE){
	ledSetColorRGB(1,0,0,0);
	chThdSleepMilliseconds(500);
	ledSetColorRGB(1,0,255,0);
	chThdSleepMilliseconds(500);
    }

    return 0;
}
