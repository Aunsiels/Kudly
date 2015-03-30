/**
 * \file hug_sensors.c
 * \brief Programme de tests.
 * \author Kudly project
 * \version 0.1
 * \date 30/03/2015
 *
 * This program controls hugs sensors
 *
 */

#include "ch.h"
#include "hal.h"
#include "hug_sensors.h"

#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      4

EVENTSOURCE_DECL(eventHugSensors);
hugSensorsValues * hugSensors;

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

static const ADCConversionGroup adcgrpcfg1 = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  NULL,
  NULL,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11 | ADC_CHANNEL_IN12)
};

void initHugSensors(void) { 
  adcStart(&ADCD1, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  chThdSleepMilliseconds(1000);
}

static WORKING_AREA(waThreadHugSensors, 128);
static msg_t ThreadHugSensors(void *arg) {
  while (TRUE) {
    hugSensors->hugSensor1 = (uint16_t) ((samples1[0] + samples1[2] + samples1[4] + samples1[6])/4);
    hug_value2->hugSensor1 = (uint16_t)(samples1[1] + samples1[3] + samples1[5] + samples1[7])/4;
    chSysLockFromIsr();
    chEvtBroadcastI(&eventHugSensors);
    chSysUnlockFromIsr();
    chThdSleepMilliseconds(*arg);
  }
  return 0;
}

void ThreadHugSensors(tprio_t priority, int * time){
  chThdCreateStatic(waThreadHugSensors, sizeof(waThreadHugSensors), priority, ThreadHugSensors, time);
}

