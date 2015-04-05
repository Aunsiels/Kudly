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
    //initUsbSerial();
    
    /* Initialize the shell */
    //shellPersoInit();
    
    palSetPadMode(GPIOA,0,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA,1,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA,2,PAL_MODE_OUTPUT_PUSHPULL);

    palSetPadMode(GPIOE,8,PAL_MODE_OUTPUT_PUSHPULL);
 
    codecInit();

    
    
    while(TRUE){
      chThdSleepMilliseconds(500);
      palTogglePad(GPIOA,0);
    }
    
    return 0;
}
