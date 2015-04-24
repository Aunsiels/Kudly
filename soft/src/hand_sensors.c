#include "ch.h"
#include "hal.h"
#include "hand_sensors.h"
#include "usb_serial.h"

/* Define number of channels and buffer size */
#define ADC_GRP2_NUM_CHANNELS   2
#define ADC_GRP2_BUF_DEPTH      8

static MUTEX_DECL(handMtx);

/* Samples buffer */
static adcsample_t samples1[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

/* Define number of channels and buffer size */
static const ADCConversionGroup adcgrpcfg1 = {
    FALSE,
    ADC_GRP2_NUM_CHANNELS,
    NULL,
    NULL,
    0,/* CR1 */
    ADC_CR2_SWSTART,          /* CR2 */
    ADC_SMPR1_SMP_AN14(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN15(ADC_SAMPLE_3),
    0,                        /* SMPR2 */
    ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),
    0,                        /* SQR2 */
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN14) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN15)
};

/* Init ADC conversion : analog input pins and ouput pin */
void initHandSensors(void) {
    adcStart(&ADCD2, NULL);
    palSetPadMode(GPIOC, GPIOC_HAND_SENSOR1_IN , PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOC, GPIOC_HAND_SENSOR2_IN , PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOA, GPIOA_HAND_SENSOR_OUT , PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOA, GPIOA_HAND_SENSOR_OUT);
    chMtxInit(&handMtx);
}

/* Command shell to launch ADC on two channels conversion */
void cmdHandSensors(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argc;
    (void)chp;
    (void)argv;
    palSetPad(GPIOA, GPIOA_HAND_SENSOR_OUT);
    adcConvert(&ADCD2, &adcgrpcfg1, samples1, ADC_GRP2_BUF_DEPTH);
    writeSerial("new ADC sample :hand1 = %d hand2= %d\r\n",samples1[0],samples1[1]);
    palClearPad(GPIOA, GPIOA_HAND_SENSOR_OUT);    
}

/* 
 * Return the values of the samples, the first of the msb and the second on the
 * lsb
 */

uint32_t getHandValues () {
    uint32_t result = 0;
    uint16_t * low = (uint16_t *) &result;
    uint16_t * high = low + 1;
    chMtxLock(&handMtx);
    palSetPad(GPIOA, GPIOA_HAND_SENSOR_OUT);
    adcConvert(&ADCD2, &adcgrpcfg1, samples1, ADC_GRP2_BUF_DEPTH);
    *low = samples1[0];
    *high = samples1[1];
    palClearPad(GPIOA, GPIOA_HAND_SENSOR_OUT);
    chMtxUnlock();
    return result;
}
