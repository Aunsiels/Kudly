/**
 * \file main.c
 * \brief Main program to launch Kudly applications
 * \author Kudly project
 * \version 0.1
 * \date 30/03/2015
 *
 * This program inits all features and starts them  
 *
 */

#include "ch.h"
#include "hal.h"

/**
 * \brief Entry point function
 *
 * \param void
 * \return Int, but infinite loop before
 */
int main(void) {

  halInit();
  chSysInit();

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
