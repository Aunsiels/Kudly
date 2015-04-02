#include "ch.h"
#include "hal.h"
#include "wifi.h"

char wifi_buffer[16];

static SerialConfig uartCfg = {
  115200,
  0,
  0,
  0
};

void wifiInitByUsb(void){  
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));
  sdStart(&SD3, &uartCfg);
}

void wifiWriteByUsb(char * message, int length){
  sdWrite(&SD3, (uint8_t *) message, length); 
}

void wifiStopByUsb(void){
  sdStop(&SD3);
}

void wifiReadByUsbTimeout(int timeout){
  sdReadTimeout(&SD3,(uint8_t *) wifi_buffer, sizeof(wifi_buffer), timeout);
}
