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
#include "main.h"


/*
 * GLOBALS
 */
static uint32_t colour;
static uint32_t led_count;
static uint     pio_offset;
static int      sm;
PIO             pio;


void ws2812_init(void) {

    led_count = 0;
    pio = pio1;
    pio_offset = 0;
    sm = 0;

    // Set defaults
    colour = NEO_COLOUR;

    // Set PIO output to feed the WS2182 via pin PIN_NEO_DATA
    pio_offset = pio_add_program(pio1, &ws2812_program);
    ws2812_program_init(pio1, PROBE_SM, pio_offset, PIN_NEO_DATA, 800000, true);

    // Power up the LED
    gpio_init(PIN_NEO_PWR);
    gpio_set_dir(PIN_NEO_PWR, GPIO_OUT);
    gpio_put(PIN_NEO_PWR, true);

    // Set the LED off
    ws2812_pixel(0x00);
}


void ws2812_pixel(uint32_t colour) {

    // Pi's code expects colours in GRB format;
    // GRB is irrational, so convert 'colour' from RGB
    uint32_t grb_colour = ((colour & 0xFF0000) >> 8) | ((colour & 0xFF00) << 8) | (colour & 0xFF);
    pio_sm_put_blocking(pio1, PROBE_SM, grb_colour << 8u);
}


void ws2812_set_state(bool state) {
    
    ws2812_pixel(state ? colour : 0x00);
}


void ws2812_flash(int count) {

    while (count > 0) {
        ws2812_pixel(colour);
        sleep_ms(250);
        ws2812_pixel(0x00);
        sleep_ms(250);
        count--;
    }

    sleep_ms(250);
}


void ws2812_set_colour(uint32_t new_colour) {

    colour = new_colour;
}