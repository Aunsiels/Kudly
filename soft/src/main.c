#include "ch.h"
#include "hal.h"
#include "led.h"

int main(void) {

    halInit();
    chSysInit();

    ledInit();
    ledTest();

    while (1) {
        chThdSleepMilliseconds(1000);
    }
}
