/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 a-pushkin on GitHub
 * Copyright (c) 2021 a-smittytone on GitHub
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _WS2812_H
#define _WS2812_H


/*
 *  INCLUDES
 */
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"


/*
 *  CONSTANTS
 */
#define LED_COUNT_SHIFT             14
#define LED_COUNT_MAX               5 * (1 << LED_COUNT_SHIFT)
#define PROBE_SM                    0


/*
 *  PROTOTYPES
 */
void    ws2812_init(void);
void    ws2812_pixel(uint32_t colour);
void    ws2812_flash(int count);
void    ws2812_set_colour(uint32_t new_colour);
void    ws2812_set_state(bool state);
void    ws2812_task(void);


#endif  // _WS2812_H
