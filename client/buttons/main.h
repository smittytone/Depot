/*
 * Multiple button input
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_


/*
 * INCLUDES
 */
#include <time.h>
#include "serialdriver.h"
#include "utils.h"
#include "gpio.h"


/*
 * STRUCTS
 */
typedef struct {
    uint8_t         gpio;
    bool            trigger_on_release;
    bool            set;
    bool            pressed;
    struct timespec *press;
} Button;

#endif      // _MAIN_H_
