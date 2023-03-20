/*
 * Depot RP2040 Bus Host Firmware - GPIIO functions
 *
 * @version     1.2.1
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _GPIO_HEADER_
#define _GPIO_HEADER_


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Pico SDK Includes
#include "pico/stdlib.h"
// App Includes
#include "serial.h"


/*
 * CONSTANTS
 */
#define GPIO_PIN_DIRN_BIT                       1
#define GPIO_PIN_STATE_BIT                      0
#define GPIO_PIN_MAX                            31

/*
 * STRUCTURES
 */
typedef struct {
    uint8_t     state_map[GPIO_PIN_MAX + 1];
} GPIO_State;


/*
 * PROTOTYPES
 */
bool    set_gpio(GPIO_State* gps, uint8_t* read_value, uint8_t* data);
void    clear_pin(GPIO_State* gps, uint32_t pin);
bool    is_pin_in_use_by_gpio(GPIO_State* gps, uint8_t pin);

#endif  // _GPIO_HEADER_
