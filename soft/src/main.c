#include "ch.h"
#include "hal.h"
#include "led.h"

int main(void) {

    halInit();
    chSysInit();

    ledInit();
    ledTest();

    while (TRUE) {
        chThdSleepMilliseconds(500);
    }
}
