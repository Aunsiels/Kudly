#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

static char http_get[] = "http_get kudly.herokuapp.com/pwm\r\n";
static char stream_read[] = "stream_read 0 50\r\n";

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    //Initialize shell
    shellPersoInit();

    //Initialize SD card
    //sdPersoInit();

    ledInit();
    //ledTest();
    
    wifiInitByUsart();
  
    wifiReadByUsart();

    wifiWriteByUsart(http_get, sizeof(http_get));
    wifiWriteByUsart(stream_read, sizeof(stream_read));

    chThdSleepMilliseconds(TIME_INFINITE);

    return 0;
}
