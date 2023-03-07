/*
 * Depot RP2040 Bus Host Firmware - Pimoroni Tiny 2040
 *
 * @version     1.2.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"


/*
 * ENTRY POINT
 */
int main(void) {
    // Initialise the LED
    tiny_led_init();

    // Enable STDIO and allow 2s for the board to come up
    if (stdio_usb_init()) {
        stdio_set_translate_crlf(&stdio_usb, false);;
        stdio_flush();

        // Start the loop
        // Function defined in `serial.c`
        rx_loop();

        // End
        return 0;
    }

    // Could not initialize stdio over USB,
    // so signal error (red) and end
    tiny_led_set_colour(0xFF0000);
    tiny_led_flash(10);
    tiny_led_on();
    return 1;
}
