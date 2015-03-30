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
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n){
  (void)adcp;
  (void)buffer;
  (void)n;
  hugSensors->hugSensor1 = (uint16_t) ((samples1[0] + samples1[2] + samples1[4] + samples1[6])/4);
  hugSensors->hugSensor2 = (uint16_t) ((samples1[1] + samples1[3] + samples1[5] + samples1[7])/4);
  chSysLockFromIsr();
  chEvtBroadcastI(&eventHugSensors);
  chSysUnlockFromIsr();
}
static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_GRP1_NUM_CHANNELS,
  adccallback,
  NULL,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_144) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_144),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12)
};

void initHugSensors(void) { 
  adcStart(&ADCD1, NULL);
  chThdSleepMilliseconds(1000);
}

void convertHugSensors(void){
  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
}

