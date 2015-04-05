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

  palSetPadMode(GPIOA ,GPIOA_LED1_R,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA ,GPIOA_LED1_G,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA ,GPIOA_LED1_B,PAL_MODE_OUTPUT_PUSHPULL);
  
  palSetPadMode(GPIOB ,GPIOB_LED2_G,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB ,GPIOB_LED2_B,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE ,GPIOE_LED2_R,PAL_MODE_OUTPUT_PUSHPULL);
  
  //ledInit();
  //ledTest();

  /* Initialize the serial over usb */
  // initUsbSerial();
  //Initialize shell
  //shellPersoInit();
  
  //Initialize SD card
  //sdPersoInit();
  while(TRUE){
    //    wifiInitByUsart();
    //wifiWriteByUsart(message, sizeof(message));
    //wifiReadByUsartTimeout(2000);
    //writeSerial(wifi_buffer);
    palTogglePad(GPIOA,GPIOA_LED1_R);
    palTogglePad(GPIOE,GPIOE_LED2_R);

    chThdSleepMilliseconds(1000);
    palTogglePad(GPIOA,GPIOA_LED1_R);
    palTogglePad(GPIOE,GPIOE_LED2_R);
 
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
