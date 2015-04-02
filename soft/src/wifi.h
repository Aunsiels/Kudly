#ifndef _WIFI_H_
#define _WIFI_H_

extern char wifi_buffer[];

void wifiInitByUsb(void);

void wifiWriteByUsb(char * message, int length);

void wifiStopByUsb(void);

void wifiReadByUsbTimeout(int timeout);

#endif
