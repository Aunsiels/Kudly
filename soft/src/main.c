#include "ch.h"
#include "hal.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"
#include "sccb.h"

int main(void) {

    halInit();
    chSysInit();

    ledInit();
    ledTest();

    /* Initialize the serial over usb */
    initUsbSerial();

    /* Initialize shell */
    shellPersoInit();

    /* Init sccb */
    sccbInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
