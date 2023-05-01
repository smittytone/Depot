/*
 * Depot RP2040 Bus Host Firmware - Button functions
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _BUTTON_HEADER_
#define _BUTTON_HEADER_


/*
 * INCLUDES
 */
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
// Pico SDK Includes
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/time.h"
// App Includes
#include "gpio.h"
#include "serial.h"


/*
 * STRUCTS
 */
typedef struct {
    bool            polarity;
    bool            trigger_on_release;
    bool            pressed;
    uint64_t        press_time;
} Button;

typedef struct {
    Button*         buttons[32];
    uint32_t        state;
    uint32_t        count;
} Button_State;


/*
 * PROTOTYPES
 */
bool    set_button(Button_State* bs, uint8_t* data);
bool    clear_button(Button_State* btns, uint8_t gpio_pin);
void    poll_buttons(Button_State* bs);


#endif  // _BUTTON_HEADER_
