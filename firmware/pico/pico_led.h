/*
 * Depot RP2040 Bus Host Firmware - Pi Pico LED
 *
 * @version     1.2.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _HEADER_PICO_LED_
#define _HEADER_PICO_LED_


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
