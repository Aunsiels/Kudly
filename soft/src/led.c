#include "led.h"
#include "string.h"
#include "chprintf.h"

#include <ch.h>
#include <hal.h>

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

void ledInit(void) {
    
  //Led 1 on timer 5, led 2 on timer 1
    pwmStart(&PWMD5, &pwmcfg_led1);
    pwmStart(&PWMD1, &pwmcfg_led2);

    palSetPadMode(GPIOA, GPIOA_LED1_R, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, GPIOA_LED1_G, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOA, GPIOA_LED1_B, PAL_MODE_ALTERNATE(2));

    palSetPadMode(GPIOB, GPIOB_LED2_R, PAL_MODE_ALTERNATE(1));
    palSetPadMode(GPIOB, GPIOB_LED2_G, PAL_MODE_ALTERNATE(1));
    palSetPadMode(GPIOE, GPIOE_LED2_B, PAL_MODE_ALTERNATE(1));
    
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
        pwmEnableChannel(&PWMD1, 2, r);
        pwmEnableChannel(&PWMD1, 1, g);
        pwmEnableChannel(&PWMD1, 0, b);
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
    
    while(TRUE){
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

        int i;
        for(i = 1; i<255; i=i*2){
            ledSetColorRGB(0, i, i, i);
            chThdSleepMilliseconds(300);	
        }

        for(i = 1; i<360; i++){
            ledSetColorHSV(0, i, i * 100 / 360, 100);
            chThdSleepMilliseconds(20);	
        }
        chThdSleepMilliseconds(1000);	

    }
    return 0;
}


void ledTest(void) {
    static WORKING_AREA(ledTest_wa, 128);

    chThdCreateStatic(ledTest_wa, sizeof(ledTest_wa),
            NORMALPRIO, ledTest_thd, NULL);
}

void cmdLed(BaseSequentialStream *chp, int argc, char *argv[]) {
    static int r, g, b, h, s, v;

    if(argc != 5 || !strcmp(argv[0], "--help")) {
        chprintf(chp, "Usage :\r\n");
        chprintf(chp, "\tled rgb {0|1|2} r_val g_val b_val\r\n");
        chprintf(chp, "\tled hsv {0|1|2} h_val s_val v_val\r\n");
        chprintf(chp, "\t1 or 2 selects only one led, 0 changes both leds\r\ǹ");
        return;
    }
    
    if(strcmp(argv[0], "rgb")) {
        r = atoi(argv[2]);
        g = atoi(argv[3]);
        b = atoi(argv[4]);

        if(r < 0 || g < 0 || b < 0 ||
                r > 255 || g > 255 || b > 255) {
            chprintf(chp, "Wrong parameters\n\r"); 
            return;
        }

        chprintf(chp, "Setting led value to (r,g,b) = (%d,%d,%d)", r, g, b);
        ledSetColorRGB(atoi(argv[1]), r, g, b);
        return;
    }

    if(strcmp(argv[0], "hsv")) {
        h = atoi(argv[2]);
        s = atoi(argv[3]);
        v = atoi(argv[4]);

        if(h < 0 || s < 0 || v < 0 ||
                h > 359 || s > 100 || v > 100) {
            chprintf(chp, "Wrong parameters\n\r"); 
            return;
        }
        
        chprintf(chp, "Setting led value to (h,s,v) = (%d,%d,%d)", h, s, v);
        ledSetColorHSV(atoi(argv[1]), h, s, v);
        return;
    }
}
