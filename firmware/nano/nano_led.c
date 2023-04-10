/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano RP2040 Connect
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"


Nano_LED_colour colour;


/**
 * @brief Initialise the LED's GPIO pin
 */
void nano_led_init(void) {

    // Initialise SPI link to Nina/ESP32

    // SET Nina/ESP32 GPIO pins to OUT
}


/**
 * @brief Turn the LED off.
 */
void nano_led_off(void) {

    // SET Nina/ESP32 GPIO pins to HIGH
}


/**
 * @brief Turn the LED on.
 */
void nano_led_on(void) {

    // SET Nina/ESP32 GPIO pins to LOW
}


/**
 * @brief Specify the LED's state (on or off).
 *
 * @param is_on: Turn the LED on (`true`) or off (`false`).
 */
void nano_led_set_state(bool is_on) {

    if (is_on) {
        nano_led_on();
    } else {
        nano_led_off();
    }
}


/**
 * @brief Flash the LED for the specified number of times.
 *
 * @param count: The number of blinks.
 */
void nano_led_flash(uint32_t count) {

     while (count > 0) {
        nano_led_on();
        sleep_ms(200);
        nano_led_off();
        sleep_ms(200);
        count--;
    }
}


/**
 * @brief Set the LED's colour, converting from a 24-bit RGB value
 *        to invidual 8-bit primary colour values.
 *        NOTE Sets the stored colour, but does not update the LED
 *             immediately. Call `nano_led_on()` to do so.
 *
 * @param rgb_colour: The colour as an RGB value bitfield:
 *                    R = bits 23-16, G = bits 15-8, B = bits 7-0.
 */
void nano_led_set_colour(uint32_t rgb_colour) {

    colour.blue = (rgb_colour & 0xFF);
    colour.green = ((rgb_colour & 0xFF00) >> 8);
    colour.red = ((rgb_colour & 0xFF0000) >> 16);
}
