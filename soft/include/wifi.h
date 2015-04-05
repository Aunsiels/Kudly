#ifndef _WIFI_H_
#define _WIFI_H_

extern char wifi_buffer[];

void wifiInitByUsart(void);

void wifiWriteByUsart(char * message, int length);

void wifiStopByUsart(void);

void wifiReadByUsartTimeout(int timeout);

void wifiReadByUsart(void);

void wifiUsartRead(void);

void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]);

#endif
