#include "ch.h"
#include "hal.h"
#include "hug_sensors.h"
#include "usb_serial.h"

#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      4

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
/*
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
*/
static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_GRP1_NUM_CHANNELS,
  NULL,
  NULL,
  0,/* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

void initHugSensors(void) {
  adcStart(&ADCD1, NULL);
  palSetPadMode(GPIOC, GPIOC_HUG_SENS1_IN , PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOC, GPIOC_HUG_SENS_OUT , PAL_MODE_OUTPUT_PUSHPULL);
  chThdSleepMilliseconds(1000);
}

void convertHugSensors(void){
  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
}

static msg_t hugSensors_thd(void * args) {
  (void)args;
  while(TRUE){
    palSetPad(GPIOC, GPIOC_HUG_SENS_OUT);
    adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
    writeSerial("new ADC sample : %d \r\n", (int)samples1[0]);
    chThdSleepMilliseconds(5000);
    palClearPad(GPIOC, GPIOC_HUG_SENS_OUT);
  }
  return 0;
}

void startHugSensor(void) {
  static WORKING_AREA(hugSensors_wa, 128);
  chThdCreateStatic(
		    hugSensors_wa, sizeof(hugSensors_wa),
		    NORMALPRIO, hugSensors_thd, NULL);
}
