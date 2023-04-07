/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano RP2040 Connect
 *
 * @version     1.2.2
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
#define     NINA_LED_R            18
#define     NINA_LED_G            16
#define     NINA_LED_B            17


/*
 * STRUCTURES
 */
typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} Nano_LED_colour;


/*
 * PROTOTYPES
 */
void    nano_led_init(void);
void    nano_led_off(void);
void    nano_led_on(void);
void    nano_led_set_state(bool is_on);
void    nano_led_flash(uint32_t count);


#endif  // _HEADER_PICO_LED_
