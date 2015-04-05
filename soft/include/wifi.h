#ifndef _WIFI_H_
#define _WIFI_H_

extern char wifi_buffer[];

void wifiInitByUsart(void);

void wifiStopByUsart(void);

void wifiReadByUsart(void);

void wifiWriteByUsart(char * message, int length);

void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
