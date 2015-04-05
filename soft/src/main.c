#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

char message[]="get ne i\r\n";
char message1[]="get ne d\r\n";


int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    initUsbSerial();

    //Initialize shell
    //shellPersoInit();

    //Initialize SD card
    //sdPersoInit();

    //ledInit();
    //ledTest();
    //
    
    wifiInitByUsart();
    //wifiReadByUsartTimeout(2000);
    //writeSerial(wifi_buffer);

    usartRead();

    chThdSleepMilliseconds(2000);

    while(true) {
        wifiWriteByUsart(message, sizeof(message));
        wifiWriteByUsart(message1, sizeof(message1));
        chThdSleepMilliseconds(1000);
    }

    chThdSleepMilliseconds(TIME_INFINITE);

    return 0;
}
