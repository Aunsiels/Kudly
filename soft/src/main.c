#include "ch.h"
#include "hal.h"

int main(void) {

  halInit();
  chSysInit();

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
