#include "hal.h"
#include "ch.h"
#include "sccb.h"

#define DELAY 30
#define SCCB_UNINIT 0
#define SCCB_READY  1
#define SIO_C 9
#define SIO_D 8

static int state = 0;

/*
 * Initialize the sccb
 */
void sccbInit(){
    /* Open drain mode */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetPadMode(GPIOC, SIO_C, PAL_MODE_OUTPUT_OPENDRAIN);
    /* Initializes the value of the pins */
    palSetPad(GPIOC, SIO_C);
    palSetPad(GPIOC, SIO_D);
    /* Ready to be used */
    state = SCCB_READY;
}

/*
 * Sends the begin of transimission sequence
 */
void sccbStartTransmission() {
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* A transmission begins with SIO_C and SIO_D both high and then both low */
    palSetPad(GPIOC, SIO_D);
    /* start condition setup time, min 600ns */
    chThdSleepMicroseconds(DELAY);

    palSetPad(GPIOC, SIO_C);
    /* t high, min 600ns */
    chThdSleepMicroseconds(DELAY);

    palClearPad(GPIOC, SIO_D);
    /* start condition hold time, min 600ns */
    chThdSleepMicroseconds(DELAY);

    palClearPad(GPIOC, SIO_C);
    /* t low, min 1.3us */
    chThdSleepMicroseconds(DELAY);
}

/*
 * Sends the end of transmission sequence
 */
void sccbStopTransimission(){
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* An end is SIO_D low and then both SIO_C and SIO_D high */
    palClearPad(GPIOC, SIO_D);
    chThdSleepMicroseconds(DELAY);

    palSetPad(GPIOC, SIO_C);
    /* Stop condition setup time */
    chThdSleepMicroseconds(DELAY);

    palSetPad(GPIOC,SIO_D);
    /* Bus free time before new start */
    chThdSleepMicroseconds(DELAY);
}

/*
 * Send Acknowledgement after reading
 */
void sccbNA(){
    /* Not initialized yet */
    if(state != SCCB_READY) return;

    /* Shows that data have been received */
    palSetPad(GPIOC, SIO_D);
    chThdSleepMicroseconds(DELAY);

    /* TIC */
    palSetPad(GPIOC, SIO_C);
    chThdSleepMicroseconds(DELAY);

    /* TAC */
    palClearPad(GPIOC, SIO_C);
    chThdSleepMicroseconds(DELAY);

    /* 
     * When there is an Acknoledgement, it will be the end of the transmission
     * because we acknowledge only the there is a read answer 
     */
    palClearPad(GPIOC, SIO_D);
    chThdSleepMicroseconds(DELAY);
}

/*
 * Send a byte to the camera
 */
int sccbSendByte(uint8_t data){
    /* Not initialized yet */
    if(state != SCCB_READY) return;
    
    int i, success;
    /* We send all the bits, beginning the MSB*/
    for(i = 0; i < 8; ++i){
        if((data << i) & 0x80)
            palSetPad(GPIOC, SIO_D);
        else
            palClearPad(GPIOC, SIO_D);
        chThdSleepMicroseconds(DELAY);
        
        /* TIC */
        palSetPad(GPIOC, SIO_C);
        chThdSleepMicroseconds(DELAY);

        /* TAC */
        palClearPad(GPIOC, SIO_C);
        chThdSleepMicroseconds(DELAY);
    }

    /* Now we check if the camera received something */
    /* We get ready to ready what is on SIO_D */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_INPUT);
    chThdSleepMicroseconds(DELAY);

    /* TIC */
    palSetPad(GPIOC, SIO_C);
    chThdSleepMicroseconds(DELAY);

    /* We read the acknowledgement */
    success = 1 - palReadPad(GPIOC, SIO_D);    

    /* TAC */
    palClearPad(GPIOC, SIO_C);
    chThdSleepMicroseconds(DELAY);

    /* We put the output mode again */
    palSetPadMode(GPIOC, SIO_D, PAL_MODE_OUTPUT_OPENDRAIN);

    return success;
}
