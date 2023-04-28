/*
 * HT16K33 8x8 matrix driver
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _HT16K33_MATRIX_HEADER_
#define _HT16K33_MATRIX_HEADER_


/*
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <inttypes.h>
// FROM 1.1.2
#include <limits.h>


/*
 * CONSTANTS
 */
#define     HT16K33_I2C_ADDR                0x70
#define     HT16K33_CMD_POWER_ON            0x21
#define     HT16K33_CMD_POWER_OFF           0x20
#define     HT16K33_CMD_DISPLAY_ON          0x81
#define     HT16K33_CMD_DISPLAY_OFF         0x80
#define     HT16K33_CMD_BRIGHTNESS          0xE0

#define     HT16K33_0_DEG                   0
#define     HT16K33_90_DEG                  1
#define     HT16K33_180_DEG                 2
#define     HT16K33_270_DEG                 3


/*
 * PROTOTYPES
 */
void        HT16K33_init(SerialDriver* sd, I2CData* i2c, uint8_t angle);
void        HT16K33_power(bool is_on);
void        HT16K33_set_angle(uint8_t angle);
void        HT16K33_draw(bool do_stop);
void        HT16K33_clear_buffer(void);
void        HT16K33_set_brightness(uint8_t brightness);
void        HT16K33_plot(uint8_t x, uint8_t y, bool is_set);
void        HT16K33_print(const char *text, uint32_t delay_ms);
void        HT16K33_rotate(uint8_t angle);
void        HT16K33_set_char(uint8_t ascii, bool is_centred);
void        HT16K33_set_glyph(uint8_t* bytes);


#endif  // _HT16K33_MATRIX_HEADER_
