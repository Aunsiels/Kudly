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
    
    ledInit();

    codecInit();

    while(TRUE){
	ledSetColorRGB(1,0,0,0);
	chThdSleepMilliseconds(500);
	ledSetColorRGB(1,0,255,0);
	chThdSleepMilliseconds(500);
    }

    return 0;
}
