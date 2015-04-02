#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"

static SerialConfig uartCfg = {
  115200,
  0,
  0,
  0
};

void wifiInitByUsart(void){  
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_TX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RX, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_CTS, PAL_MODE_ALTERNATE(7));
  palSetPadMode (GPIOD,GPIOD_WIFI_UART_RTS, PAL_MODE_ALTERNATE(7));
  sdStart(&SD3, &uartCfg);
}

void wifiWriteByUsart(char * message, int length){
  sdWrite(&SD3, (uint8_t *) message, length); 
}

void wifiStopByUsart(void){
  sdStop(&SD3);
}

void wifiReadByUsartTimeout(int timeout){
  sdReadTimeout(&SD3,(uint8_t *) wifi_buffer, strlen(wifi_buffer), timeout);
}
