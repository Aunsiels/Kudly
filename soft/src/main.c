#include "ch.h"
#include "hal.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"
#include "sccb.h"
#include "dcmi.h"

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    /* Initialize shell */
    shellPersoInit();

    /* Initialize SD card */
    sdPersoInit();

    ledInit();
    ledTest();

    /* Camera pins PWDN */
    palSetPadMode(GPIOC, 14, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOC, 14);

    /* Camera pin */
    palSetPadMode(GPIOA, GPIOA_CAMERA_HSYNC, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOA, GPIOA_CAMERA_PIXCLK, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D0, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D1, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOC, GPIOC_CAMERA_D4, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D5, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_VSYNC, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D6, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOB, GPIOB_CAMERA_D7, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, GPIOE_CAMERA_D2, PAL_MODE_ALTERNATE(13));
    palSetPadMode(GPIOE, GPIOE_CAMERA_D3, PAL_MODE_ALTERNATE(13));
    /* XCLK */
    palSetPadMode(GPIOA, GPIOA_CAMERA_XCLK, PAL_MODE_ALTERNATE(0) | PAL_STM32_OSPEED_HIGHEST);

    chThdSleepMilliseconds(100);
    palClearPad(GPIOC, 14);

    /* Init sccb */
    sccbInit();

    /* DCMI init */
    dcmiInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
