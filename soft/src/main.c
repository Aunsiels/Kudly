#include "ch.h"
#include "hal.h"

#include "led.h"

int main(void) {

  halInit();
  chSysInit();

  ledSetColorRGB(0,255, 0, 0);

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
