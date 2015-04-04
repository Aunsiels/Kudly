#include "hal.h"
#include "ch.h"
#include "sccb.h"

#define DELAY
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


