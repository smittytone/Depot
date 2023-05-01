/*
 * Depot RP2040 Bus Host Firmware - GPIO functions
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "gpio.h"


/**
 * @brief Set or get a GPIO pin.
 *
 * @param gps:        The GPIO state record.
 * @param read_value: Pointer to a byte into which to read the pin value, if read.
 * @param data:       The command data.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool set_gpio(GPIO_State* gps, uint8_t* read_value, uint8_t* data) {

    uint8_t gpio_pin = (data[1] & 0x1F);
    bool pin_state   = (data[1] & 0x80);
    bool is_dir_out  = (data[1] & 0x40);
    bool is_read     = (data[1] & 0x20);

    // NOTE Function will not have been called if a bus is using the pin,
    //      but the check should really be here

    // Register pin usage, state and initialise if necessary
    if (gps->state_map[gpio_pin] == 0x00) {
        gpio_init(gpio_pin);
        gpio_set_dir(gpio_pin, (is_dir_out ? GPIO_OUT : GPIO_IN));

        if (is_dir_out) {
        gps->state_map[gpio_pin] |= (1 << GPIO_PIN_DIRN_BIT);
        gps->state_map[gpio_pin] |= (1 << GPIO_PIN_DIRN_BIT);
        gps->state_map[gpio_pin] |= (1 << GPIO_PIN_STATE_BIT);
            gps->state_map[gpio_pin] |= (1 << GPIO_PIN_DIRN_BIT);
        gps->state_map[gpio_pin] |= (1 << GPIO_PIN_STATE_BIT);
        } else {
            // FROM 1.2.3
            // Set the pin to revert to GND
            gpio_pull_down(gpio_pin);
        }

        if (pin_state) gps->state_map[gpio_pin] |= (1 << GPIO_PIN_STATE_BIT);
    } else {
        // FROM 1.2.3
        // Pin registered: check for a direction change
        bool current_dir = (gps->state_map[gpio_pin] & (1 << GPIO_PIN_DIRN_BIT));
        if (current_dir != is_dir_out) {
            gpio_set_dir(gpio_pin, (is_dir_out ? GPIO_OUT : GPIO_IN));
            gps->state_map[gpio_pin] ^= (1 << GPIO_PIN_STATE_BIT);
        }
    }

    if (is_read && !is_dir_out) {
        // Pin is DIGITAL_IN, so get and return the state
        uint8_t pin_value = gpio_get(gpio_pin) ? 0x80 : 0x00;
        *read_value = (pin_value | gpio_pin);

#ifdef DO_UART_DEBUG
        debug_log("Pin %i read value: %i", gpio_pin, *read_value);
#endif

        return true;
    } else if (is_dir_out) {
        // Pin is DIGITAL_OUT, so just set the state
        gpio_put(gpio_pin, pin_state);

#ifdef DO_UART_DEBUG
        debug_log("Pin %i state set: %i", gpio_pin, (pin_state ? 1 : 0));
#endif

        return true;
    } else {
        // Pin is DIGITAL_IN, but we're just setting it

#ifdef DO_UART_DEBUG
        debug_log("Pin %i set to input", gpio_pin);
#endif

        return true;
    }

    return false;
}


/**
 * @brief Clear a pin's usage record.
 *
 * @param gps: The GPIO state record.
 * @param pin: An arbitrary GPIO pin that we're checking.
 */
void clear_pin(GPIO_State* gps, uint32_t pin) {

    gpio_deinit(pin);
    gps->state_map[pin] = 0x00;
}


/**
 * @brief Check pin usage.
 *
 * @param gps: The GPIO state record.
 * @param pin: An arbitrary GPIO pin that we're checking.
 *
 * @returns `true` if the pin is in use by the bus, or `false`.
 */
bool is_pin_in_use_by_gpio(GPIO_State* gps, uint8_t pin) {

    return (gps->state_map[pin] != 0x00);
}
