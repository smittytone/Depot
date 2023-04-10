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
#define     NINA_LED_R            18
#define     NINA_LED_G            16
#define     NINA_LED_B            17

#define     NINA_CMD_SET_DIGITAL_WRITE      00
#define     NINA_CMD_SET_PIN_MODE           11
#define     NINA_CMD_END                    22

#define     NINA_PIN_RSTN                   3
#define     NINA_PIN_GPIO0                  2
#define     NINA_PIN_READY                  10
#define     NINA_PIN_SPI_CS                 9
#define     NINA_PIN_SPI_TX                 11
#define     NINA_PIN_SPI_RX                 8
#define     NINA_PIN_SPI_SCK                14

#define     NINA_SPI                        spi1


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
void    nano_led_set_colour(uint32_t rgb_colour);


#endif  // _HEADER_PICO_LED_
