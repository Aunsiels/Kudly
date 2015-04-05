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
    
    codecInit();

    palSetPadMode(GPIOA,0,PAL_MODE_OUTPUT_PUSHPULL);
    
    while(TRUE){
      chThdSleepMilliseconds(500);
      palTogglePad(GPIOA,0);
    }
    
    return 0;
}
