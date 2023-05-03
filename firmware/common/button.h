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
 * CONSTANTS
 */
#define         BUTTON_STATE_READY                  00
#define         BUTTON_STATE_WAITING_RELEASE        1

#define         BUTTON_DEBOUNCE_PERIOD_US           10000


/*
 * STRUCTS
 */
typedef struct {
    bool            polarity;
    bool            trigger_on_release;
    bool            pressed;
    uint32_t        press_time;
} Button;

typedef struct {
    Button*         buttons[32];
    uint32_t        states;
    uint32_t        count;
} Button_State;



/*
 * PROTOTYPES
 */
bool    set_button(Button_State* bts, uint8_t* data);
bool    clear_button(Button_State* bts, uint8_t pin);
void    poll_buttons(Button_State* bts);
bool    is_pin_in_use_by_button(Button_State* bts, uint8_t pin);

#endif  // _BUTTON_HEADER_
