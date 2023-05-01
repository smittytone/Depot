/*
 * Depot RP2040 Bus Host Firmware - Button functions
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "button.h"


/**
 * @brief Configure a button.
 *
 * @param btns: The button state record.
 * @param data: The data received from the host.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool set_button(Button_State* btns, uint8_t* data) {

    // Extract info from the payload
    // Bits 0-4 - RP2040 GPIO pin number
    // Bit 5    - Is the op a read? Not used here
    // Bit 6    - The button fires when released (1) or pressed (0)
    // Bit 7    - The button falls to HIGH (1) or LOW (0)
    uint8_t gpio = (data[1] & 0x1F);
    bool trigger_on_release = (data[1] & 0x40);
    bool polarity = (data[1] & 0x80);

    // Create a new button
    Button* btn = (Button*)malloc(sizeof(Button));
    btn->trigger_on_release = trigger_on_release;
    btn->polarity = polarity;
    btn->pressed = false;
    btn->press_time = -1;

    // Replacing an existing button? Zap it
    if (btns->buttons[gpio]) free(btns->buttons[gpio]);
    btns->buttons[gpio] = btn;
    btns->count++;

    // Initialise the button's GPIO
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);

    if (polarity) {
        gpio_pull_up(gpio);
    } else {
        gpio_pull_down(gpio);
    }

    return true;
}


/**
 * @brief Poll each configured button for its state.
 *        TODO Make this interrupt driven
 *
 * @param btns: The button state record.
 */
void poll_buttons(Button_State* btns) {

    for (uint8_t i = 0 ; i < GPIO_PIN_MAX + 1 ; ++i) {
        // Get the next button to poll
        Button* btn = btns->buttons[i];
        if (btn) {
            bool pin_high = gpio_get(i);
            if (!pin_high && btn->pressed) {
                // BUTTON RELEASED
                btn->pressed = false;
                if (btn->trigger_on_release) btns->state |= (1 << (i - 0));
            } else if (pin_high) {
                // BUTTON PRESSED?
                if (btn->press_time == -1) {
                    // No press seen yet, so assume one and start the count
                    btn->press_time = time_us_64();
                } else {
                    // Button has been pressed -- check count
                    if (!btn->pressed) {
                        if (time_us_64() - btn->press_time >= 10000) {
                            // Still held after debounce period
                            btn->pressed = true;
                            btn->press_time = -1;
                            if (!btn->trigger_on_release) btns->state |= (1 << (i - 0));
                        }
                    }
                }
            }
        }
    }
}


/**
 * @brief Clear a button.
 *
 * @param btns:     The button state record.
 * @param gpio_pin: Pointer to a byte into which to read the pin value, if read.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool clear_button(Button_State* btns, uint8_t gpio_pin) {

    // Make sure the pin is in range
    if (gpio_pin > GPIO_PIN_MAX) return false;

    // Clear the pin
    gpio_deinit(gpio_pin);

    // Zap the button
    free(btns->buttons[gpio_pin]);
    btns->buttons[gpio_pin] = NULL;
    btns->count--;
}
