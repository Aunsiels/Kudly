/**
 * \file hug_sensors.c
 * \brief Programme de tests.
 * \author Kudly project
 * \version 0.1
 * \date 30/03/2015
 *
 * This program controls hugs sensonrs
 *
 */

#include "ch.h"
#include "hal.h"
#include "addpwm.h"
#include "trimmer.h"

#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      4

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
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10)
};

void initTrimmer(void) {
  palSetPadMode(GPIOC, GPIOC_TRIM, PAL_MODE_INPUT_ANALOG);
  
  adcStart(&ADCD1, NULL);
  
  adcStartConversion(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  chThdSleepMilliseconds(1000);
}

static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {
  (void)arg;
  while (TRUE) {
    startpwm(2,((uint32_t)samples1[0])/4);
    chThdSleepMilliseconds(50);
  }
  return 0;
}
void trimmerToLed(tprio_t priority){
  chThdCreateStatic(waThread1, sizeof(waThread1), priority, Thread1, NULL);
}

