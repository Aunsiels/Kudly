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
 * \brief Sets the color of led 1
 * \param r   The amount of red, from 0 to 255
 * \param g   The amount of green, from 0 to 255
 * \param b   The amount of blue, from 0 to 255
 */
void ledSetColor1(uint8_t r, uint8_t g, uint8_t b);

/**
 * \brief Sets the color of led 2
 * \param r   The amount of red, from 0 to 255
 * \param g   The amount of green, from 0 to 255
 * \param b   The amount of blue, from 0 to 255
 */
void ledSetColor2(uint8_t r, uint8_t g, uint8_t b);

/**
 * \brief Toggles led 1
 */
void ledToggle1(void);

/**
 * \brief Toggles led 2
 */
void ledToggle2(void);

/**
 * \brief Testing leds thread
 */
void ledTest(void);

#endif
