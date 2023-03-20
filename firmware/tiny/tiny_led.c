/*
 * Depot RP2040 Bus Host Firmware - Tiny 2040 LED
 *
 * @version     1.2.1
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"


Tiny_LED_colour colour;


/**
 * @brief Initialise the LED's GPIO pins
 */
void tiny_led_init(void) {

    // Initialise the RGB pins' PWM drivers
    tiny_pwm_init(PIN_TINY_LED_BLUE);
    tiny_pwm_init(PIN_TINY_LED_GREEN);
    tiny_pwm_init(PIN_TINY_LED_RED);

    // Set the LED's colour
    tiny_led_set_colour(DEFAULT_LED_COLOUR);
}


/**
 * @brief Initialise the LED's GPIO pins
 *
 * @param pin: The GPIO pin to provision.
 */
void tiny_pwm_init(uint pin) {

    // Set the pin's function
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Configure and enable PWM
    uint slice =  pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);
    pwm_set_wrap(slice, 1000);                  // The frequency, basically
    pwm_set_chan_level(slice, channel, 65535);  // Channel levels 0-65535
                                                // The Tiny's LED is common cathode,
                                                // so level 65535 is lowest brightness;
                                                // level 0 is max brightness.
    pwm_set_enabled(slice, true);
}


/**
 * @brief Turn the LED off.
 */
void tiny_led_off(void) {

    tiny_led_set_state(false);
}


/**
 * @brief Turn the LED on.
 */
void tiny_led_on(void) {

    tiny_led_set_state(true);
}


/**
 * @brief Specify the LED's state (on or off).
 *
 * @param is_on: Turn the LED on (`true`) or off (`false`).
 */
void tiny_led_set_state(bool is_on) {

    if (is_on) {
        pwm_set_gpio_level(PIN_TINY_LED_BLUE, ((0xFF - colour.blue) / 0xFF) * 65535);
        pwm_set_gpio_level(PIN_TINY_LED_GREEN, ((0xFF - colour.green) / 0xFF) * 65535);
        pwm_set_gpio_level(PIN_TINY_LED_RED, ((0xFF - colour.red) / 0xFF) * 65535);
    } else {
        pwm_set_gpio_level(PIN_TINY_LED_BLUE, 65535);
        pwm_set_gpio_level(PIN_TINY_LED_GREEN, 65535);
        pwm_set_gpio_level(PIN_TINY_LED_RED, 65535);
    }
}


/**
 * @brief Flash the LED for the specified number of times.
 *
 * @param count: The number of blinks.
 */
void tiny_led_flash(uint32_t count) {

     while (count > 0) {
        tiny_led_set_state(true);
        sleep_ms(200);
        tiny_led_set_state(false);
        sleep_ms(200);
        count--;
    }
}


/**
 * @brief Set the LED's colour, converting from a 24-bit RGB value
 *        to invidual 8-bit primary colour values.
 *
 * @param rgb_colour: The colour as an RGB value bitfield:
 *                    R = bits 23-16, G = bits 15-8, B = bits 7-0.
 */
void tiny_led_set_colour(uint32_t rgb_colour) {

    colour.blue = (rgb_colour & 0xFF);
    colour.green = ((rgb_colour & 0xFF00) >> 8);
    colour.red = ((rgb_colour & 0xFF0000) >> 16);
}
