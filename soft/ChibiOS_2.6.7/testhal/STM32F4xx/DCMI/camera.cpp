/*
 * camera.cpp - contains implementation of functions used with
 * Toshiba TCM8230MD camera module on the Camera board.
 * Author: Ewan Coldicott
 * Date: 11 June 2012
 * Modified: 14 Jan 2013
 */
 
#include "ch.h"
#include "hal.h"
#include "camera.h"
#include "chprintf.h"

using namespace std;
 

 /*
 * PWM configuration structure, for Timer4 (camera EXTCLK).
 * Cyclic callback disabled.
 * Channel 3 enabled without callback.
 * The active state is a logic one.
 */
static PWMConfig pwm4cfg = {
  24000000,                                 /* 24MHz PWM clock frequency. Max clock is 84MHz. */
  2,                                        /* PWM period every 2 ticks (ie new period @ 12MHz, D = 0, 0.5 possible) */
  NULL,                                     /* No cyclic callback */
  {
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},         /* Channel 3 enabled, active high output. No callback. */
    {PWM_OUTPUT_DISABLED, NULL}
  },
  /* HW dependent part.*/
  0
};


 /*
 * I2C2 driver configuration structure.
 */
static const I2CConfig i2c2cfg = {
  OPMODE_I2C,
  400000,
  STD_DUTY_CYCLE,
};

 
 /*
 * Camera start and test code.
 */
void cameraInit(void) {
  /* Start up required drivers */
  pwmStart(&PWMD4, &pwm4cfg);
  i2cStart(&I2CD2, &i2c2cfg);
  
  pwmEnableChannel(&PWMD4, 2, 1);     /* Enable PWM output on channel 3, width 1 clock tick -> D=0.5, 12MHz square wave */
  chThdSleepMilliseconds(1);          /* Wait for greater than 100 clock ticks */
  palSetPad(GPIOD, GPIOD_RESET);      /* Pull RESET line high in order to enable camera */  
  chThdSleepMilliseconds(1);          /* Wait for camera to start */
}


/*
 *  Configures camera to have desired image settings:
 *  - 160x120, RGB565
 *  - Test pattern (colour bars) DISABLED
 *  - Embedded synchronisation codes DISABLED
 *  - Output ENABLED
 */
void cameraConfigure(void) {
  systime_t tmo = MS2ST(20);
  uint8_t txbuf[2];

  txbuf[0] = CAM_CONFIG1;
  txbuf[1] =
    CAM_CONFIG1_FPS;        /* FPS bit high = 15fps for 'normal' EXTCLK */
  i2cAcquireBus( &I2CD2 );
  i2cMasterTransmitTimeout( &I2CD2, CAM_ADDR, txbuf, 2, NULL, 0, tmo );
  i2cReleaseBus( &I2CD2 );
  chThdSleepMilliseconds(1);

  txbuf[0] = CAM_CONFIG3;
  txbuf[1] =
    CAM_CONFIG3_TESPIC |
    CAM_CONFIG3_HSYNCSEL |  /* HD blanking */ 
    CAM_CONFIG3_D_MASK_0 ;
  i2cAcquireBus( &I2CD2 );
  i2cMasterTransmitTimeout( &I2CD2, CAM_ADDR, txbuf, 2, NULL, 0, tmo );
  i2cReleaseBus( &I2CD2 );
  chThdSleepMilliseconds(1);

  txbuf[0] = CAM_CONFIG2;
  txbuf[1] =
    CAM_CONFIG2_PICFMT   |  /* PICFMT = 1 -> RGB565 output. PICFMT = 0 -> YUV422 */
//    CAM_CONFIG2_PICSIZ_0 |  /* 160x120 */
//    CAM_CONFIG2_PICSIZ_1 |
    CAM_CONFIG2_PICSIZ_3 ;
  i2cAcquireBus( &I2CD2 );
  i2cMasterTransmitTimeout( &I2CD2, CAM_ADDR, txbuf, 2, NULL, 0, tmo );
  i2cReleaseBus( &I2CD2 );
}


/*
 *      Camera I2C communication test code. Prints result on Serial Driver 3
 */
void cameraI2CTest(void) {        
  msg_t status = RDY_OK;
  systime_t tmo = MS2ST(10);

  /* Camera should be ready to respond to requests: test it. */
  /* The camera should acknowledge the I2C transaction */
  uint8_t txbuf[2];
  txbuf[0] = CAM_CONFIG1;         
  txbuf[1] = CAM_CONFIG1_DEFAULT; 
  i2cAcquireBus( &I2CD2 );        
  status = i2cMasterTransmitTimeout( &I2CD2, CAM_ADDR, txbuf, 2, NULL, 0, tmo );
  i2cReleaseBus( &I2CD2 );
  
  if( status == RDY_RESET ) {     
    //chprintf((BaseChannel*)&SD3, "Camera I2C test: Error %X\n\r", i2cGetErrors(&I2CD2));
  }
  else if( status == RDY_TIMEOUT ) {
    //chprintf((BaseChannel*)&SD3, "Camera I2C test: Timeout\n\r");
  }
  else {
 //   chprintf((BaseChannel*)&SD3, "Camera I2C test: Success\n\r");
  }
}

