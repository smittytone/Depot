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
#include "hardware/spi.h"


/*
 * CONSTANTS
 */
// Nina own pins
#define     NINA_LED_R                      27 //18 Should these be GPIO numbers (used) or pin numbers? (comment values)
#define     NINA_LED_G                      25 //16
#define     NINA_LED_B                      26 //17

// Nina firmware commands
#define     NINA_CMD_SET_PIN_MODE           0x50
#define     NINA_CMD_DIGITAL_WRITE          0x51
#define     NINA_CMD_ANALOG_WRITE           0x52
#define     NINA_CMD_START                  0xE0
#define     NINA_CMD_END                    0xEE

#define     NINA_OUT                        1
#define     NINA_IN                         0
#define     NINA_HIGH                       1
#define     NINA_LOW                        0

// Nina RP2040 pins and buses
#define     NINA_PIN_GPIO0                  2
#define     NINA_PIN_RSTN                   3
#define     NINA_PIN_READY                  10
#define     NINA_PIN_SPI_CS                 9
#define     NINA_PIN_SPI_RX                 8
#define     NINA_PIN_SPI_TX                 11
#define     NINA_PIN_SPI_SCK                14
#define     NINA_SPI                        spi1

#define     PIN_MONO_LED                    6


/*
 * PROTOTYPES
 */
void    nano_led_init(void);
void    nano_led_off(void);
void    nano_led_on(void);
void    nano_led_set_state(bool is_on);
void    nano_led_flash(uint32_t count);
void    nano_led_set_colour(uint32_t rgb_colour);


#endif  // _HEADER_PICO_LED_
