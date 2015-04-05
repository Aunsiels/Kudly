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

  ledInit();
  ledTest();

  /* Initialize the serial over usb */
  initUsbSerial();
  //Initialize shell
  shellPersoInit();
  
  //Initialize SD card
  sdPersoInit();
  while(TRUE){
    wifiInitByUsart();
    wifiWriteByUsart(message, sizeof(message));
    wifiReadByUsartTimeout(2000);
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
