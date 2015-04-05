#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

char wifi_buffer[16];
char message[]="get wlan.ssid\r\n";

int main(void) {

    halInit();
    chSysInit();

    /* Initialize the serial over usb */
    //initUsbSerial();

    //Initialize shell
    //shellPersoInit();

    //Initialize SD card
    //sdPersoInit();

    ledInit();
    ledTest();
    
    //wifiInitByUsart();
    //wifiWriteByUsart(message, sizeof(message));
    //wifiReadByUsartTimeout(2000);
    //writeSerial(wifi_buffer);

    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
