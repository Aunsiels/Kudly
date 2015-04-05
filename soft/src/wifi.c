#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"
#include "usb_serial.h"


char wifi_buffer[256];
static SerialConfig uartCfg =
{
  115200,
  0,
  0,
0// bit rate
};

void wifiInitByUsart(void){  
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));
  sdStart(&SD3, &uartCfg);
}

void wifiWriteByUsart(char * message, int length){
  sdWrite(&SD3, (uint8_t*)message, length); 
}

void wifiStopByUsart(void){
  sdStop(&SD3);
}

void wifiReadByUsartTimeout(int timeout){
  (void)timeout;
  sdReadTimeout(&SD3,(uint8_t *) wifi_buffer, 256, timeout);
  writeSerial(wifi_buffer);
}

void wifiReadByUsart(void){
  sdRead(&SD3,(uint8_t *) wifi_buffer, 256);
}
