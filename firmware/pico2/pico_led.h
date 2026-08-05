/*
 * Depot RP2040 Bus Host Firmware - Pi Pico LED
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#ifndef _HEADER_PICO_LED_
#define _HEADER_PICO_LED_


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdint.h>


/*
 * CONSTANTS
 */
#define     PIN_PICO_LED            25


/*
 * PROTOTYPES
 */
void    pico_led_init(void);
void    pico_led_off(void);
void    pico_led_on(void);
void    pico_led_set_state(bool is_on);
void    pico_led_flash(uint32_t count);


#endif  // _HEADER_PICO_LED_
