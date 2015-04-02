#include "ch.h"
#include "hal.h"
#include "wifi.h"


char wifi_buffer[16];
char message[]="get wlan.ssid\r\n";

int main(void) {

  halInit();
  chSysInit();

  wifiInitByUsart();
  wifiWriteByUsart(message, sizeof(message));
  wifiReadByUsartTimeout(2000);

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
