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

    /* Camera pin */
    palSetPadMode(GPIOA, 4, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, 0, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, 1, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(0));

    /* Camera pins PWDN */
    palSetPadMode(GPIOC, 14, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOC, 14);
    chThdSleepMilliseconds(100);
    palClearPad(GPIOC, 14);

    /* Init sccb */
    sccbInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
