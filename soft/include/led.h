/**
 * \file led.h
 * \brief Contains functions to control LEDs.
 */
#ifndef LED_H
#define LED_H

/**
 * \brief Need to be called before using leds
 */
void ledInit();

/**
 * \brief Sets the color of leds
 * \param led Set to 1 or 2 to select the led, 0 sets the color for both leds 1 & 2
 * \param r   The amount of red, from 0 to 255
 * \param g   The amount of green, from 0 to 255
 * \param b   The amount of blue, from 0 to 255
 */
void setColor(int led, uint8_t r, uint8_t g, uint8_t b);

/**
 * \brief Toggles LEDs
 * \param led The led to toggle (1 or 2), or 0 to toggle both leds
 */
void toggleLed(int led);

#endif
