/**
 * \file led.h
 * \brief Contains functions to control LEDs.
 */
#ifndef LED_H
#define LED_H

#include <ch.h>
#include <hal.h>

/**
 * \brief Need to be called before using leds
 */
void ledInit(void);

/**
 * \brief RGB color
 * \param led Led selector. If 0, both LED colors are changed
 * \param r   The amount of red, from 0 to 255
 * \param g   The amount of green, from 0 to 255
 * \param b   The amount of blue, from 0 to 255
 *
 * Sets the RGB color of LEDs
 */
void ledSetColorRGB(int led, int r, int g, int b);

/**
 * \brief HSV color
 * \param led Led selector. If 0, both LED colors are changed
 * \param h   Hue, from 0 to 359
 * \param s   Saturation, from 1 to 100
 * \param v   Value, from 1 to 100
 *
 * Sets the Hue-Saturation-Value color of LEDs
 */
void ledSetColorHSV(int led, int h, int s, int v);

/**
 * \brief Testing leds thread
 */
void ledTest(void);

void cmdLed(BaseSequentialStream *chp, int argc, char *argv[]);
void cmdLedtest(BaseSequentialStream *chp, int argc, char *argv[]);


#endif
