#include "ch.h"
#include "hal.h"
#include "wifi.h"
#include "string.h"

static UARTConfig uartCfg = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
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
  uartStart(&UARTD3, &uartCfg);
}

void wifiWriteByUsart(char * message, int length){
  uartStartSend(&UARTD3, length,message); 
}

void wifiStopByUsart(void){
  // sdStop(&SD3);
}

void wifiReadByUsartTimeout(int timeout){
  (void)timeout;
  //sdReadTimeout(&SD3,(uint8_t *) wifi_buffer, strlen(wifi_buffer), timeout);
}

void wifiReadByUsart(void){
  uartStartReceive(&UARTD3, 7 , wifi_buffer); 
  //  sdRead(&SD3,(uint8_t *) wifi_buffer, strlen(wifi_buffer));
}
