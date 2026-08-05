/*
 * Depot RP2040 Bus Host Firmware - Raspberry Pi Pico
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */

// C
#include <stdbool.h>
// Pico
#include "pico/stdlib.h"
// Depot
#include "../common/serial.h"
#ifdef NEO_BUILD
#include "../common/ws2812.h"
#else
#include "pico_led.h"
#endif

/*
 * ENTRY POINT
 */
int main(void) {
    // Initialise the LED
#ifdef NEO_BUILD
    ws2812_init();
#else
    pico_led_init();
    pico_led_off();
#endif

    // Enable STDIO and allow 2s for the board to come up
    if (stdio_usb_init()) {
        stdio_set_translate_crlf(&stdio_usb, false);
        stdio_flush();

        // Start the loop
        // Function defined in `serial.c`
        rx_loop();

        // End
        return 0;
    }

    // Could not initialize stdio over USB,
    // so signal error and end
#ifdef NEO_BUILD
    ws2812_set_colour(0xFF0000);
    ws2812_flash(10);
    ws2812_pixel(0xFF0000);
#else
    pico_led_flash(10);
    pico_led_on();
#endif
    return 1;
}
