#include "ch.h"
#include "hal.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"
#include "codec.h"

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    /* Initialize the shell */
    shellPersoInit();

    sdPersoInit();

    palSetPadMode(GPIOA,0,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA,1,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA,2,PAL_MODE_OUTPUT_PUSHPULL);

    palSetPadMode(GPIOE,8,PAL_MODE_OUTPUT_PUSHPULL);

    codecInit();
    palSetPad(GPIOA,0);
    chThdSleepMilliseconds(2000);
    palClearPad(GPIOA,0);
    chThdSleepMilliseconds(500);
    codecPlayMusic("testa.ogg");
    //codecEncodeSound(10000,"az.ogg");


    while(TRUE){
      chThdSleepMilliseconds(500);
      palTogglePad(GPIOA,1);
    }

    return 0;
}
