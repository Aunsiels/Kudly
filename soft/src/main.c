#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

char message[]="set wlan.ssid \"54vergniaud\"\r\n";
char message1[]="set wlan.passkey \"rose2015rulez\"\r\n";
char message2[]="save\r\n";


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
    
    wifiInitByUsart();
    //wifiReadByUsartTimeout(2000);
    //writeSerial(wifi_buffer);

    
    wifiWriteByUsart(message, sizeof(message));
    //writeSerial(message);
    wifiReadByUsartTimeout(2000);

    chThdSleepMilliseconds(2000);
    wifiWriteByUsart(message1, sizeof(message1));
    wifiReadByUsartTimeout(2000);

    chThdSleepMilliseconds(2000);
    wifiWriteByUsart(message2, sizeof(message2));
    wifiReadByUsartTimeout(2000);
    chThdSleepMilliseconds(2000);

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
