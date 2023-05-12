/*
 * Generic macOS I2C driver
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _I2C_DRIVER_H
#define _I2C_DRIVER_H


/*
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include "serialdriver.h"
#include "utils.h"



/*
 * CONSTANTS
 */
#define PREFIX_BYTE_READ                0x80
#define PREFIX_BYTE_WRITE               0xC0

#define EXIT_OK                         0
#define EXIT_ERR                        1

#define HOST_INFO_BUFFER_MAX_B          129
#define CONNECTED_DEVICES_MAX_B         120
#define SCAN_BUFFER_MAX_B               512

#define ACK                             0x0F
#define ERR                             0xF0


/*
 * STRUCTURES
 */
typedef struct {
    unsigned int    speed;              // I2C line speed (in kHz)
    uint8_t         address;            // I2C address
} I2CData;


/*
 * PROTOTYPES
 */
// Setup
bool            i2c_init(SerialDriver *sd);
bool            i2c_deinit(SerialDriver *sd);
bool            i2c_set_speed(SerialDriver *sd, long speed);
bool            i2c_set_bus(SerialDriver *sd, uint8_t bus_id, uint8_t sda_pin, uint8_t scl_pin);
bool            i2c_reset(SerialDriver *sd);

// Information
void            i2c_get_info(SerialDriver *sd, bool do_print);
void            i2c_scan(SerialDriver *sd);

// I2C operations
bool            i2c_start(SerialDriver *sd, uint8_t address, uint8_t op);
bool            i2c_stop(SerialDriver *sd);

// Data transfer
size_t          i2c_write(SerialDriver *sd, const uint8_t bytes[], size_t nn);
void            i2c_read(SerialDriver *sd, uint8_t bytes[], size_t nn);



#endif  // I2C_DRIVER_H
