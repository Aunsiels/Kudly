#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "led.h"
#include "usb_serial.h"
#include "shell_cfg.h"
#include "sd_perso.h"

char wifi_buffer[7];
char message[]="\r\n";

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


    while(TRUE){
      chThdSleepMilliseconds(2000);
      wifiWriteByUsart(message, sizeof(message));
      writeSerial(message);
      wifiReadByUsart();
      (SDU1.vmt)->write(&SDU1, (uint8_t*)wifi_buffer, 7);
      writeSerial("\r\n");
    }
    chThdSleepMilliseconds(TIME_INFINITE);
    return 0;
}
