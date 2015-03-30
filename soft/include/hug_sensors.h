#ifndef _HUG_SENSORS_H_
#define _HUG_SENSORS_H_

extern EventSource eventHugSensors;

void initHugSensors(void);
void convertHugSensors(void);

typedef struct {
  uint16_t hugSensor1;
  uint16_t hugSensor2;
} hugSensorsValues;

extern hugSensorsValues * hugSensors;

#endif
