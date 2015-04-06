#ifndef _WIFI_H_
#define _WIFI_H_

void wifiInitByUsart(void);

void wifiStopByUsart(void);

void wifiReadByUsart(void);

void wifiWriteByUsart(char * message, int length);

void cmdWifi(BaseSequentialStream *chp, int argc, char *argv[]);

void wifiCommands(void);
#endif
