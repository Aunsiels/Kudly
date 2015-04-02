#include "ch.h"
#include "hal.h"
#include "wifi.h"

int main(void) {

  halInit();
  chSysInit();

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
