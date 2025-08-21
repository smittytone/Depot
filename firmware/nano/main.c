/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano RP2040 Connect
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2025
 * @licence     MIT
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Pico
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
// Depot
#include "nano_led.h"
#include "../common/serial.h"


/*
 * ENTRY POINT
 */
int main(void) {
    // Initialise the LED
    nano_led_init();
    nano_led_off();

    // Enable STDIO and allow 2s for the board to come up
    if (stdio_usb_init()) {
        stdio_set_translate_crlf(&stdio_usb, false);
        stdio_flush();

        // Start the loop
        // Function defined in `serial.c`
        rx_loop();

        // End
        // return 0;
    }

    // Could not initialize stdio over USB,
    // so signal error and end
    nano_led_flash(10);
    nano_led_on();
    return 1;
}
