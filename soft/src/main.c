#include "ch.h"
#include "hal.h"

int main(void) {

  halInit();
  chSysInit();

  palSetPadMode(GPIOA,0,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA,1,PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA,2,PAL_MODE_OUTPUT_PUSHPULL);

  palSetPad(GPIOA,0);
  palSetPad(GPIOA,1);
  palSetPad(GPIOA,2);

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
