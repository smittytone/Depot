/*
 * Depot RP2040 Bus Host Firmware - Tiny 2040 LED
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _HEADER_TINY_LED_
#define _HEADER_TINY_LED_


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Pico SDK Includes
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"


/*
 * CONSTANTS
 */
#define     PIN_TINY_LED_BLUE           20
#define     PIN_TINY_LED_GREEN          19
#define     PIN_TINY_LED_RED            18

#define     DEFAULT_LED_COLOUR          0x001010    // Cyan


/*
 * STRUCTURES
 */
typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} Tiny_LED_colour;


/*
 * PROTOTYPES
 */
void    tiny_led_init(void);
void    tiny_pwm_init(uint pin);
void    tiny_led_off(void);
void    tiny_led_on(void);
void    tiny_led_set_state(bool is_on);
void    tiny_led_flash(uint32_t count);
void    tiny_led_set_colour(uint32_t rgb_colour);


#endif  // _HEADER_TINY_LED_
