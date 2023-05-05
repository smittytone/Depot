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
 * @param btn_states: The button state record.
 * @param data: The data received from the host.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool set_button(Button_State* bts, uint8_t* data) {

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
    btn->press_time = BUTTON_STATE_READY;

    // Replacing an existing button? Zap it
    if (bts->buttons[gpio]) free(bts->buttons[gpio]);
    bts->buttons[gpio] = btn;
    bts->count++;

    // Initialise the button's GPIO
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);

    if (polarity) {
        gpio_pull_up(gpio);
    } else {
        gpio_pull_down(gpio);
    }

#ifdef DO_UART_DEBUG
        debug_log("Button %i set (pull %s, trigger: %s)", gpio, (polarity ? "UP" : "DN"), (trigger_on_release ? "REL" : "PRESS"));
#endif

    return true;
}


/**
 * @brief Poll each configured button for its state.
 *        TODO Make this interrupt driven
 *
 * @param bts: The button state record.
 */
void poll_buttons(Button_State* bts) {

    // Check each button
    for (uint8_t i = 0 ; i < GPIO_PIN_MAX + 1 ; ++i) {
        // Get the next button to poll
        Button* btn = bts->buttons[i];
        if (btn) {
            uint32_t now = time_us_32();
            bool is_pin_high = gpio_get(i);
            // Respect the btn's polarity setting
            if (btn->polarity ? is_pin_high : !is_pin_high) {
                if (btn->press_time == BUTTON_STATE_READY) {
                    // Set debounce timer
                    btn->press_time = now;
                } else if (now - btn->press_time > 5000) {
                    btn->press_time = BUTTON_STATE_READY;
                    btn->pressed = true;
                    if (!btn->trigger_on_release) bts->states |= (1 << (i - 1));
                }
            } else if (btn->pressed) {
                if (btn->trigger_on_release) bts->states |= (1 << (i - 1));
                btn->pressed = false;
            }
        }
    }
}


/**
 * @brief Clear a button.
 *
 * @param bts: The button state record.
 * @param pin: Pointer to a byte into which to read the pin value, if read.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool clear_button(Button_State* bts, uint8_t pin) {

    // Make sure the pin is in range
    if (pin > GPIO_PIN_MAX) return false;

    // Clear the pin
    gpio_deinit(pin);

    // Zap the button
    free(bts->buttons[pin]);
    bts->buttons[pin] = NULL;
    bts->count--;
}


/**
 * @brief Check pin usage.
 *
 * @param gps: The GPIO state record.
 * @param pin: An arbitrary GPIO pin that we're checking.
 *
 * @returns `true` if the pin is in use by the bus, or `false`.
 */
bool is_pin_in_use_by_button(Button_State* bts, uint8_t pin) {

    return (bts->buttons[pin] != NULL);
}
