/*
   Original File license:
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
    
   Code below is adapted from the STM32F4 Discovery board demo for Chibi/OS which has the above license.
*/

#include "ch.h"
#include "hal.h"
#include "camera.h"
#include "bmp.h"
#include "chprintf.h"

using namespace std;

const uint32_t IMG_HEIGHT = 96;
const uint32_t IMG_WIDTH  = 128;
const uint32_t BPP        = 2;       //bytes per pixel
const uint32_t IMG_SIZE = IMG_HEIGHT*IMG_WIDTH*BPP;

uint8_t imgBuf[IMG_SIZE];
uint8_t* imgBuf0 = imgBuf;
uint8_t* imgBuf1 = &(imgBuf[IMG_SIZE/2]);

EventSource es1, es2;
EventListener el1, el2;

static volatile uint32_t frameEndCbCount = 0;
static volatile uint32_t dmaTxferEndCbCount = 0;

void frameEndCb(DCMIDriver* dcmip) {
   (void) dcmip;
   chEvtBroadcastI(&es2);
   frameEndCbCount++;
}

void dmaTxferEndCb(DCMIDriver* dcmip) {
   (void) dcmip;
   chEvtBroadcastI(&es1);
   dmaTxferEndCbCount++;
}

static const DCMIConfig dcmicfg = {
   frameEndCb,
   dmaTxferEndCb,
   0            //empty cr
};

/*
 * Serial driver 3 configuration structure.
 * Uses USART3, provides buffers, nice abstraction etc.
 * 57600bps, 8N1
 */
static const SerialConfig sd3cfg = {
  57600,                    /* Bitrate */
  0,                        /* USART3 config register 1: defines parity settings etc */
  USART_CR2_STOP1_BITS,     /* Config register 2: defines stop bits */
  0                         /* Config register 3 */
};


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * an LED.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOA, GPIOA_LED2);       /* User LED */
    chThdSleepMilliseconds(10);
    palClearPad(GPIOA, GPIOA_LED2); 
   // chprintf((BaseSequentialStream*)&SD3,"DMA: %d Frame: %d\r\n", dmaTxferEndCbCount, frameEndCbCount);
    chThdSleepMilliseconds(1000);
  }
  return (msg_t) 0;
}


static WORKING_AREA(waThread4, 2048);
static msg_t Thread4(void *arg) {
  (void) arg;
  chRegSetThreadName("encoder-transmitter");
  chEvtRegisterMask(&es1, &el1, EVENT_MASK(1));
  chEvtRegisterMask(&es2, &el2, EVENT_MASK(2));
  while(TRUE) {
    chThdSleepMilliseconds(10000);
    //chprintf((BaseSequentialStream*)&SD3, "Beginning transfer:\n\r");
    //using synchronous API for simplicity, single buffer.
    //limits max image size to available SRAM. Note that max DMA transfers in one go is 65535.
    // i.e. IMG_SIZE cannot be larger than 65535 here.
    dcmiStartReceiveOneShot(&DCMID1, IMG_SIZE/2, imgBuf0, imgBuf1);
    chEvtWaitOne(EVENT_MASK(1));
    chEvtWaitOne(EVENT_MASK(1)); //| EVENT_MASK(2));
    chThdSleepMilliseconds(1);
    //chprintf((BaseSequentialStream*)&SD3, "%u DMAs, %u Frames, dumping BMP:\n\r", dmaTxferEndCbCount, frameEndCbCount );
    palSetPad(GPIOA, GPIOA_LED2);
    send16bppBmpImage( (BaseSequentialStream*)&SD3, (uint16_t*)imgBuf, IMG_WIDTH, IMG_HEIGHT );
    palClearPad(GPIOA, GPIOA_LED2);
  }
  
  return 0;   //will never execute
}

/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  chEvtInit(&es1);
  chEvtInit(&es2);

  /* Activates the serial driver 3 using the config structure defined above.
   * PC10(TX) and PC11(RX) are routed to USART3 in board.h */
  sdStart(&SD3, &sd3cfg);
 
  dcmiStart(&DCMID1, &dcmicfg);
 
  /* Initialises the I2C2 peripheral, tests for connectivity, enables camera
   * Defined in camera.h */
  cameraInit();
  cameraI2CTest();
  cameraConfigure();

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread4, sizeof(waThread4), NORMALPRIO, Thread4, NULL);

  /*
   * Normal main() thread activity. Do nothing.
   */
 while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
