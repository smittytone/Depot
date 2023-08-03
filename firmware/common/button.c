/*
 * Depot RP2040 Bus Host Firmware - Button functions
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "button.h"


static void     button_debounce(uint gpio, uint32_t event_mask);
static int64_t  button_get_state(alarm_id_t id, void *user_data);

volatile Button_State* button_state;


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
    button_state = bts;

    // Replacing an existing button? Zap the earlier one
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

    // Set the trigger callback
    uint32_t event_mask = polarity ? GPIO_IRQ_EDGE_FALL : GPIO_IRQ_EDGE_RISE;
    gpio_set_irq_enabled_with_callback(gpio, event_mask, true, &button_debounce);

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
    for (size_t i = 1 ; i < GPIO_PIN_MAX + 1 ; ++i) {
        // Get the next button to poll
        Button* btn = bts->buttons[i];
        if (btn) {
            uint32_t now = time_us_32();
            bool is_pin_pushed = gpio_get(i);
            // Respect the btn's polarity setting
            is_pin_pushed = (btn->polarity ? !is_pin_pushed : is_pin_pushed);
            if (is_pin_pushed) {
                if (!btn->pressed) {
                    if (btn->press_time == BUTTON_STATE_READY) {
                        // Set debounce timer
                        btn->press_time = now;
                    } else if (now - btn->press_time > 5000) {
                        btn->press_time = BUTTON_STATE_READY;
                        btn->pressed = true;
                        // Set the button state record (1 = pressed)
                        if (!btn->trigger_on_release) bts->states |= (1 << (i - 1));
                    }
                }
            } else if (btn->pressed) {
                // Button released: set the button state record
                btn->pressed = false;
                if (btn->trigger_on_release) {
                    // Set released trigger-on-release pin
                    bts->states |= (1 << (i - 1));
                }
            }
        }
    }
}


/**
 * @brief Clear a button.
 *        NOTE Button's pin value already checked.
 *
 * @param bts: The button state record.
 * @param pin: Pointer to a byte into which to read the pin value, if read.
 *
 * @returns Whether the operation was successful (`true`) or not (`false`).
 */
bool clear_button(Button_State* bts, uint8_t pin) {

    // Zap the button
    if (bts->buttons[pin]) {
        free(bts->buttons[pin]);
        bts->buttons[pin] = NULL;
        bts->count--;

        // Clear the pin
        gpio_deinit(pin);
        return true;
    } 
    
    // Button not yet set
    return false;
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


static void button_debounce(uint gpio, uint32_t event_mask) {

    // Remove IRQ from pin
    gpio_set_irq_enabled(gpio, event_mask, false);

    // Wait 0.10ms and call the button state handler
    add_alarm_in_us(1000, &button_get_state, &gpio, true);

#ifdef DO_UART_DEBUG
    debug_log("DEBOUNCE");
#endif 
}


static int64_t button_get_state(alarm_id_t id, void *user_data) {

    // Dereference the `user_data` pointer to get the GPIO
    // number and thus the button record for this pin
    uint32_t* gpio_ptr = (uint32_t*)user_data;
    uint32_t gpio = *gpio_ptr;
    Button* btn = button_state->buttons[gpio];

    if (btn->polarity == gpio_get(gpio)) {
        // Pin is back to normal: release
        if (btn->trigger_on_release) {
            // Set released trigger-on-release pin
            button_state->states |= (1 << (gpio - 1));
        }
    } else {
        // Button pressed
        if (!btn->trigger_on_release) {
            // Set released trigger-on-release pin
            button_state->states |= (1 << (gpio - 1));
        }
    }

    // Re-enable IRQ
    uint32_t event_mask = btn->polarity ? GPIO_IRQ_EDGE_FALL : GPIO_IRQ_EDGE_RISE;
    gpio_set_irq_enabled(gpio, event_mask, true);

#ifdef DO_UART_DEBUG
    debug_log("GET_STATE");
#endif

    return 0;
}
