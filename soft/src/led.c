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

    palSetPadMode(GPIOA, 0, PIN_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, 1, PIN_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, 2, PIN_MODE_ALTERNATE(2));

    palSetPadMode(GPIOE, 8, PIN_MODE_ALTERNATE(1));
    palSetPadMode(GPIOB, 0, PIN_MODE_ALTERNATE(1));
    palSetPadMode(GPIOB, 1, PIN_MODE_ALTERNATE(1));

    pwmEnableChannel(&PWMD5, 0, 0);
    pwmEnableChannel(&PWMD5, 1, 0);
    pwmEnableChannel(&PWMD5, 2, 0);

    pwmEnableChannel(&PWMD1, 0, 0);
    pwmEnableChannel(&PWMD1, 1, 0);
    pwmEnableChannel(&PWMD1, 2, 0);
}

void ledSetColorRGB(int led, int r, int g, int b) {
    if(led == 1 || led == 0) {
        pwmEnableChannel(&PWMD5, 0, r);
        pwmEnableChannel(&PWMD5, 1, g);
        pwmEnableChannel(&PWMD5, 2, b);
    } 
    
    if(led == 2 || led == 0) {
        pwmEnableChannel(&PWMD1, 0, r);
        pwmEnableChannel(&PWMD1, 1, g);
        pwmEnableChannel(&PWMD1, 2, b);
    }
}

void ledSetColorHSV(int led, int h, int s, int v) {
    int r = 0, g = 0, b = 0;

    int hueSection = h / 60;
    int f = (h - hueSection * 60);

    int val = v * 255 / 100; //Normalized value
    int l = v * 255 * (100 - s) / 10000;
    int m = v * 255 / 100 - v * 255 * s * f / 60 / 10000;
    int n = v * 255 / 100 - v * 255 * s * (60 - f) / 60 / 10000;

    switch(hueSection) {
        case 0:
            r = val; g = n; b = l; break;
        case 1:
            r = m; g = val; b = l; break;
        case 2:
            r = l; g = val; b = n; break;
        case 3:
            r = l; g = m; b = val; break;
        case 4:
            r = n; g = l; b = val; break;
        case 5:
            r = val; g = l; b = m; break;
    }

    ledSetColorRGB(led, r, g, b);
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

    ledSetColorRGB(0, 0, 0, 0);

    chThdSleepMilliseconds(500);

    ledSetColorRGB(1, 255, 0, 0);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(1, 0, 255, 0);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(1, 0, 0, 255);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(1, 0, 0, 0);

    ledSetColorRGB(2, 255, 0, 0);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(2, 0, 255, 0);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(2, 0, 0, 255);
    chThdSleepMilliseconds(400);
    ledSetColorRGB(2, 0, 0, 0);

    ledSetColorHSV(1, 255, 0, 0);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(1, 0, 255, 0);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(1, 0, 0, 255);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(1, 0, 0, 0);

    ledSetColorHSV(2, 255, 0, 0);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(2, 0, 255, 0);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(2, 0, 0, 255);
    chThdSleepMilliseconds(400);
    ledSetColorHSV(2, 0, 0, 0);

    chThdSleep(TIME_INFINITE);
    return 0;
}


void ledTest(void) {
    static WORKING_AREA(ledTest_wa, 128);

    chThdCreateStatic(ledTest_wa, sizeof(ledTest_wa),
            NORMALPRIO, ledTest_thd, NULL);
}
