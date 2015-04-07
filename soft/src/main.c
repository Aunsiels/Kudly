#include "ch.h"
#include "hal.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    //Initialize shell
    shellPersoInit();

    //Initialize SD card
    sdPersoInit();

    ledInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
