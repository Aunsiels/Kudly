#include "led.h"
#include "string.h"
#include "chprintf.h"
#include <stdlib.h>
#include <ch.h>
#include <hal.h>
#include "usb_serial.h"

/* PWM configuration for led 1 */
static PWMConfig pwmcfg_led1 = {
    100000,
    256,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0,
    0
};

/* PMW configuration for led 2 */
static PWMConfig pwmcfg_led2 = {
    100000,
    256,
    NULL,
    {
        { PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, NULL},
        { PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, NULL},
        { PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0,
    0
};

/*
 * Initializes the leds
 */
void ledInit(void) {

    //Led 1 on timer 5, led 2 on timer 1
    pwmStart(&PWMD5, &pwmcfg_led1);
    pwmStart(&PWMD1, &pwmcfg_led2);

    /* Configure pin mode */
    palSetPadMode(GPIOA, GPIOA_LED1_R, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, GPIOA_LED1_G, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, GPIOA_LED1_B, PAL_MODE_ALTERNATE(2));

    palSetPadMode(GPIOB, GPIOB_LED2_R, PAL_MODE_ALTERNATE(1));
    palSetPadMode(GPIOB, GPIOB_LED2_G, PAL_MODE_ALTERNATE(1));
    palSetPadMode(GPIOE, GPIOE_LED2_B, PAL_MODE_ALTERNATE(1));

    /* Turn off the leds */
    pwmEnableChannel(&PWMD5, 0, 0);
    pwmEnableChannel(&PWMD5, 1, 0);
    pwmEnableChannel(&PWMD5, 2, 0);

    pwmEnableChannel(&PWMD1, 0, 0);
    pwmEnableChannel(&PWMD1, 1, 0);
    pwmEnableChannel(&PWMD1, 2, 0);
}

/*
 * Set the rgv color
 */
void ledSetColorRGB(int led, int r, int g, int b) {
    /* 0 is for the two leds */
    if(led == 1 || led == 0) {
        pwmEnableChannel(&PWMD5, 0, r);
        pwmEnableChannel(&PWMD5, 1, g);
        pwmEnableChannel(&PWMD5, 2, b);
    }

    if(led == 2 || led == 0) {
        pwmEnableChannel(&PWMD1, 2, r);
        pwmEnableChannel(&PWMD1, 1, g);
        pwmEnableChannel(&PWMD1, 0, b);
    }
}

/*
 * Same than set RGB color with HSV
 */
void ledSetColorHSV(int led, int h, int s, int v) {
    int r = 0, g = 0, b = 0;

    /* Calculate the rgb values from the HSV one */
    int hueSection = h / 60;
    int f = (h - hueSection * 60);

    int val = v * 255 / 100; //Normalized value
    int l = v * 255 * (100 - s) / 10000;
    int m = v * 255 / 100 - v * 255 * s * f / 60 / 10000;
    int n = v * 255 / 100 - v * 255 * s * (60 - f) / 60 / 10000;

    switch(hueSection) {
    case 0:
        r = val;
        g = n;
        b = l;
        break;
    case 1:
        r = m;
        g = val;
        b = l;
        break;
    case 2:
        r = l;
        g = val;
        b = n;
        break;
    case 3:
        r = l;
        g = m;
        b = val;
        break;
    case 4:
        r = n;
        g = l;
        b = val;
        break;
    case 5:
        r = val;
        g = l;
        b = m;
        break;
    }

    /* We use the set rgb function */
    ledSetColorRGB(led, r, g, b);
}

/* A thread that test the leds */
static msg_t ledTest_thd(void * args) {
    (void)args;

    /* A test sequence */
    while(TRUE) {
        ledSetColorRGB(0, 255, 255, 255);
        chThdSleepMilliseconds(500);
        ledSetColorRGB(0, 0, 0, 0);
        chThdSleepMilliseconds(500);

        ledSetColorRGB(1, 255, 0, 0);
        chThdSleepMilliseconds(300);
        ledSetColorRGB(1, 0, 255, 0);
        chThdSleepMilliseconds(300);
        ledSetColorRGB(1, 0, 0, 255);
        chThdSleepMilliseconds(300);

        ledSetColorRGB(2, 255, 0, 0);
        chThdSleepMilliseconds(300);
        ledSetColorRGB(2, 0, 255, 0);
        chThdSleepMilliseconds(300);
        ledSetColorRGB(2, 0, 0, 255);
        chThdSleepMilliseconds(300);
        ledSetColorRGB(2, 0, 0, 0);
        chThdSleepMilliseconds(300);

        ledSetColorHSV(1, 0, 10, 100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(1, 120, 100, 100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(1, 240, 100, 100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(1, 0, 0, 0);
        chThdSleepMilliseconds(300);

        ledSetColorHSV(2, 0, 100,100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(2, 120, 100, 100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(2, 240, 100, 100);
        chThdSleepMilliseconds(300);
        ledSetColorHSV(2, 0, 0, 0);


    }
    return 0;
}

/*
 * Create the thread for the test led
 */
void ledTest(void) {
    static WORKING_AREA(ledTest_wa, 128);

    chThdCreateStatic(ledTest_wa, sizeof(ledTest_wa),
                      NORMALPRIO, ledTest_thd, NULL);
}

void cmdLedtest(BaseSequentialStream *chp, int argc, char * argv[]) {

    writeSerial( "Starting test leds...\n\r");
    (void)argv;
    (void)argc;
    (void)chp;

    ledSetColorRGB(0, 255, 255, 255);
    chThdSleepMilliseconds(500);
    ledSetColorRGB(0, 0, 0, 0);
    chThdSleepMilliseconds(500);

    ledSetColorRGB(1, 255, 0, 0);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(1, 0, 255, 0);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(1, 0, 0, 255);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(1, 0, 0, 0);

    ledSetColorRGB(2, 255, 0, 0);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(2, 0, 255, 0);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(2, 0, 0, 255);
    chThdSleepMilliseconds(300);
    ledSetColorRGB(2, 0, 0, 0);

    ledSetColorHSV(1, 0, 10, 100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(1, 120, 100, 100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(1, 240, 100, 100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(1, 0, 0, 0);

    ledSetColorHSV(2, 0, 100,100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(2, 120, 100, 100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(2, 240, 100, 100);
    chThdSleepMilliseconds(300);
    ledSetColorHSV(2, 0, 0, 0);

    int i;
    for(i = 1; i<255; i += 5) {
        ledSetColorRGB(0, i, i, i);
        chThdSleepMilliseconds(20);
    }

    ledSetColorRGB(0, 0, 0, 0);

    for(i = 1; i<360; i++) {
        ledSetColorHSV(0, i, i * 100 / 360, 100);
        chThdSleepMilliseconds(20);
    }

    ledSetColorRGB(0, 0, 0, 0);
}

void cmdLed(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)chp;
    static int r, g, b, h, s, v, led;

    if(argc != 5 || !strcmp(argv[0], "--help")) {
        writeSerial( "Usage :\r\n");
        writeSerial( "\tled rgb {0|1|2} r_val g_val b_val\r\n");
        writeSerial( "\tled hsv {0|1|2} h_val s_val v_val\r\n");
        writeSerial( "\t1 or 2 selects only one led, 0 changes both leds\r\n");
        return;
    }

    if(!strcmp(argv[0], "rgb")) {
        r = strtol(argv[2], (char **)NULL, 10);
        g = strtol(argv[3], (char **)NULL, 10);
        b = strtol(argv[4], (char **)NULL, 10);
        led = strtol(argv[1], (char **)NULL, 10);

        if(r < 0 || g < 0 || b < 0 ||
                r > 255 || g > 255 || b > 255) {
            writeSerial( "Wrong parameters\n\r");
            return;
        }

        writeSerial( "Setting led value to (r,g,b) = (%d,%d,%d)\n\r", r, g, b);
        ledSetColorRGB(led, r, g, b);
        return;
    }

    if(!strcmp(argv[0], "hsv")) {
        h = strtol(argv[2], (char **)NULL, 10);
        s = strtol(argv[3], (char **)NULL, 10);
        v = strtol(argv[4], (char **)NULL, 10);
        led = strtol(argv[1], (char **)NULL, 10);

        if(h < 0 || s < 0 || v < 0 ||
                h > 359 || s > 100 || v > 100) {
            writeSerial( "Wrong parameters\n\r");
            return;
        }

        writeSerial( "Setting led value to (h,s,v) = (%d,%d,%d)\n\r", h, s, v);
        ledSetColorHSV(led, h, s, v);
        return;
    }
}
