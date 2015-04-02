#include "ch.h"
#include "hal.h"

#include "usb_serial.h"
#include "shell_cfg.h"
#include "ff.h"
#include "sd_perso.h"

/*
 * Application entry point.
 */
int main(void) {

    //Initialize the system
    halInit();
    chSysInit();

    //Initialize the serial over usb
    init_usb_serial();
    if (SDU2.state == SDU_READY){
            palTogglePad(GPIOF, GPIOF_CAM_PWR);
    }

    //Initialize shell
    shell_init();

    //Initialize SD card
    sdInit();

    //Wait infinitely
    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
