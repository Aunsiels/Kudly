#include "ch.h"
#include "hal.h"

int main(void) {

  halInit();
  chSysInit();

  /* Led used for debug */
  palSetPadMode(GPIOA,0,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA,1,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA,2,PAL_MODE_OUTPUT_PUSHPULL);

  palSetPad(GPIOA,0);
  palClearPad(GPIOA,1);
  palClearPad(GPIOA,2);

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
