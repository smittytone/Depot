/*
 * Depot RP2040 Bus Host Firmware - LED control middleware
 *
 * @version     1.2.1
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _HEADER_LED_
#define _HEADER_LED_


/*
 * CONSTANTS
 */
#ifdef NEO_BUILD
#include "../qtpy/ws2812.h"
#endif

#ifdef LED_BUILD
#include "../pico/pico_led.h"
#endif

#ifdef TINY_BUILD
#include "../tiny/tiny_led.h"
#endif


/*
 * PROTOTYPES
 */
void led_off(void);
void led_on(void);
void led_set_state(bool is_on);
void led_flash(uint32_t count);
void led_set_colour(uint32_t colour);


#endif  // _HEADER_LED_
