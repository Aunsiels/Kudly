#include "led.h"

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
      pwmEnableChannel(&PWMD5, 0, 255);
      pwmEnableChannel(&PWMD5, 1, 255);
      pwmEnableChannel(&PWMD5, 2, 255);
      
      pwmEnableChannel(&PWMD1, 0, 255);
      pwmEnableChannel(&PWMD1, 1, 255);
      pwmEnableChannel(&PWMD1, 2, 255);
      
      chThdSleepMilliseconds(1000);
      
      ledSetColorRGB(0, 0, 0, 0);
      
      chThdSleepMilliseconds(1000);
      
      ledSetColorRGB(1, 255, 0, 0);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(1, 0, 255, 0);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(1, 0, 0, 255);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(1, 0, 0, 0);
      chThdSleepMilliseconds(1000);
      
      ledSetColorRGB(2, 255, 0, 0);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(2, 0, 255, 0);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(2, 0, 0, 255);
      chThdSleepMilliseconds(1000);
      ledSetColorRGB(2, 0, 0, 0);
      chThdSleepMilliseconds(1000);
      
      ledSetColorHSV(1, 0, 100, 100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(1, 120, 100, 100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(1, 240, 100, 100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(1, 0, 0, 0);
      chThdSleepMilliseconds(1000);
      
      ledSetColorHSV(2, 0, 100,100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(2, 120, 100, 100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(2, 240, 100, 100);
      chThdSleepMilliseconds(1000);
      ledSetColorHSV(2, 0, 0, 0);

      int i;
      for(i = 1; i<255; i=i*2){
	ledSetColorRGB(0, i, i, i);
	chThdSleepMilliseconds(1000);	
      }
      
      for(i = 1; i<360; i++){
	ledSetColorHSV(0, i, 100, 100);
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

