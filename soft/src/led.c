#include "led.h"

#include <ch.h>
#include <hal.h>

static PWMConfig pwmcfg_led1 = {
    10000,
    256,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0
};

static PWMConfig pwmcfg_led2 = {
    10000,
    256,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0
};

void ledInit(void) {
    //Led 1 on timer 5, led 2 on timer 1
    pwmStart(&PWMD5, &pwmcfg_led1);
    pwmStart(&PWMD1, &pwmcfg_led2);

    pwmEnableChannel(&PWMD5, 0, 0);
    pwmEnableChannel(&PWMD5, 1, 0);
    pwmEnableChannel(&PWMD5, 2, 0);

    pwmEnableChannel(&PWMD1, 0, 0);
    pwmEnableChannel(&PWMD1, 1, 0);
    pwmEnableChannel(&PWMD1, 2, 0);
}

void ledSetColor1(uint8_t r, uint8_t g, uint8_t b) {
    pwmEnableChannel(&PWMD5, 0, r);
    pwmEnableChannel(&PWMD5, 1, g);
    pwmEnableChannel(&PWMD5, 2, b);
}

void ledSetColor2(uint8_t r, uint8_t g, uint8_t b) {
    pwmEnableChannel(&PWMD1, 0, r);
    pwmEnableChannel(&PWMD1, 1, g);
    pwmEnableChannel(&PWMD1, 2, b);
}

void ledToggle1(void) {
    pwmEnableChannel(&PWMD5, 0, 0);
    pwmEnableChannel(&PWMD5, 1, 0);
    pwmEnableChannel(&PWMD5, 2, 0);
}

void ledToggle2(void) {
    pwmEnableChannel(&PWMD1, 0, 0);
    pwmEnableChannel(&PWMD1, 1, 0);
    pwmEnableChannel(&PWMD1, 2, 0);
}

static msg_t ledTest_thd(void * args) {
    (void)args;
    pwmEnableChannel(&PWMD5, 0, 255);
    pwmEnableChannel(&PWMD5, 1, 255);
    pwmEnableChannel(&PWMD5, 2, 255);

    pwmEnableChannel(&PWMD1, 0, 255);
    pwmEnableChannel(&PWMD1, 1, 255);
    pwmEnableChannel(&PWMD1, 2, 255);

    chThdSleepMilliseconds(500);

    pwmEnableChannel(&PWMD5, 0, 0);
    pwmEnableChannel(&PWMD5, 1, 0);
    pwmEnableChannel(&PWMD5, 2, 0);

    pwmEnableChannel(&PWMD1, 0, 0);
    pwmEnableChannel(&PWMD1, 1, 0);
    pwmEnableChannel(&PWMD1, 2, 0);

    chThdSleepMilliseconds(500);

    setColor_led1(255, 0, 0);
    chThdSleepMilliseconds(400);
    setColor_led1(0, 255, 0);
    chThdSleepMilliseconds(400);
    setColor_led1(0, 0, 255);
    chThdSleepMilliseconds(400);
    setColor_led1(0, 0, 0);

    setColor_led2(255, 0, 0);
    chThdSleepMilliseconds(400);
    setColor_led2(0, 255, 0);
    chThdSleepMilliseconds(400);
    setColor_led2(0, 0, 255);
    chThdSleepMilliseconds(400);
    setColor_led2(0, 0, 0);

    chThdSleep(TIME_INFINITE);
    return 0;
}


void ledTest(void) {
    static WORKING_AREA(ledTest_wa, 128);

    chThdCreateStatic(ledTest_wa, sizeof(ledTest_wa),
            NORMALPRIO, ledTest_thd, NULL);
}
