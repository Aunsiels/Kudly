#include "ch.h"
#include "hal.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

int main(void) {

    halInit();
    chSysInit();

    ledInit();
    ledTest();
    /*
    palSetPadMode(GPIOB,0, PAL_MODE_OUTPUT_PUSHPULL);


    while(TRUE){
      palTogglePad(GPIOB,0);
      chThdSleepMilliseconds(1000);
      palTogglePad(GPIOB,0);
      chThdSleepMilliseconds(1000);
      }*/
    /* Initialize the serial over usb */
    //initUsbSerial();

    //Initialize shell
    //shellPersoInit();

    //Initialize SD card
    //sdPersoInit();

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
