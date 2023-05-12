/*
 * macOS/Linux Depot GPIO Functions
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _GPIO_FUNCTIONS_H
#define _GPIO_FUNCTIONS_H


/*
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "serialdriver.h"
#include "utils.h"


/*
 * PROTOTYPES
 */
bool        gpio_set_pin(SerialDriver *sd, uint8_t pin);
uint8_t     gpio_get_pin(SerialDriver *sd, uint8_t pin);
bool        gpio_clear_pin(SerialDriver *sd, uint8_t pin);


#endif      // _GPIO_FUNCTIONS_H
