#include "ch.h"
#include "hal.h"
#include "hug_sensors.h"
#include "usb_serial.h"

/* Define number of channels and buffer size */
#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      8

/* Samples buffer */
static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

/* Define number of channels and buffer size */
static const ADCConversionGroup adcgrpcfg1 = {
    FALSE,
    ADC_GRP1_NUM_CHANNELS,
    NULL,
    NULL,
    0,/* CR1 */
    ADC_CR2_SWSTART,          /* CR2 */
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_3),
    0,                        /* SMPR2 */
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
    0,                        /* SQR2 */
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN12)
};

/* Init ADC conversion : analog input pins and ouput pin */
void initHugSensors(void) {
    adcStart(&ADCD1, NULL);
    palSetPadMode(GPIOC, GPIOC_HUG_SENS1_IN , PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOC, GPIOC_HUG_SENS2_IN , PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOC, GPIOC_HUG_SENS_OUT , PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOC, GPIOC_HUG_SENS_OUT);
}

/* Command shell to launch ADC on two channels conversion */
void cmdHugSensors(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argc;
    (void)chp;
    (void)argv;
    palSetPad(GPIOC, GPIOC_HUG_SENS_OUT);
    adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
    writeSerial("new ADC sample :hug 1 = %d hug2= %d\r\n",samples1[0],samples1[1]);
    palClearPad(GPIOC, GPIOC_HUG_SENS_OUT);    
}
