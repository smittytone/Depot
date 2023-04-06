/*
 * Depot RP2040 Bus Host Firmware - I2C functions
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _HEADER_I2C_
#define _HEADER_I2C_


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Pico SDK Includes
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
// App Includes
#include "serial.h"


/*
 * CONSTANTS
 */
#ifndef DEFAULT_I2C_BUS
#define DEFAULT_I2C_BUS                         1
#endif


/*
 * STRUCTURES
 */
typedef struct {
    bool        is_ready;
    bool        is_started;
    bool        is_read_op;
    uint8_t     address;
    uint8_t     sda_pin;
    uint8_t     scl_pin;
    uint32_t    frequency;
    uint32_t    read_byte_count;
    uint32_t    write_byte_count;
    i2c_inst_t* bus;
} I2C_State;


/*
 * PROTOTYPES
 */
void    init_i2c(I2C_State* itr);
void    deinit_i2c(I2C_State* its);
void    reset_i2c(I2C_State* itr);
void    set_i2c_frequency(I2C_State* its, uint32_t frequency_khz);
bool    configure_i2c(I2C_State* its, uint8_t* data);
void    send_i2c_scan(I2C_State* itr);
void    send_i2c_status(I2C_State* itr);
bool    is_pin_in_use_by_i2c(I2C_State* its, uint8_t pin);


#endif  // _HEADER_LED_
