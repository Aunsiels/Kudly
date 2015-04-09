#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"
#include "sccb.h"
#include "wifi_manager.h"

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

    /* XCLK */
    palSetPadMode(GPIOA, GPIOA_CAMERA_XCLK, PAL_MODE_ALTERNATE(0));

    /* Camera pins PWDN */
    palSetPadMode(GPIOC, GPIOC_CAMERA_ENABLE, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOC, GPIOC_CAMERA_ENABLE);

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

    chThdSleepMilliseconds(100);
    palClearPad(GPIOC, GPIOC_CAMERA_ENABLE);

    /* Init sccb */
    sccbInit();

    chThdSleepMilliseconds(10000);
    
    /* Read wifi by usart */
    usartRead();

    /* Init Wifi */
    wifiInitByUsart();

    //saveWebPage( "bla", "wifi.txt");

    chThdSleepMilliseconds(TIME_INFINITE);

    return 0;
}
