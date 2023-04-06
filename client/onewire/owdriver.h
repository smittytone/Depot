/*
 * macOS/Linux Depot 1-Wire driver
 *
 * Version 1.2.2
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _ONE_WIRE_DRIVER_H_
#define _ONE_WIRE_DRIVER_H_


/*
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>

#include "serialdriver.h"
#include "utils.h"


/*
 * CONSTANTS
 */
#define     OW_CMD_SKIP_ROM                 0xCC
#define     OW_CMD_READ_ROM                 0x33
#define     OW_CMD_SEARCH_ROM               0xF0
#define     OW_CMD_MATCH_ROM                0x55


/*
 * PROTOTYPES
 */
// Setup
bool        one_wire_init(SerialDriver *sd);
bool        one_wire_reset(SerialDriver *sd);
bool        one_wire_configure_bus(SerialDriver *sd, uint8_t data_pin);

// Information
void        one_wire_get_info(SerialDriver *sd, bool do_print);
void        one_wire_scan(SerialDriver *sd);

// Convenience commands
void        one_wire_cmd_skip_rom(SerialDriver *sd);
void        one_wire_cmd_read_rom(SerialDriver *sd);
void        one_wire_cmd_search_rom(SerialDriver *sd);
void        one_wire_cmd_match_rom(SerialDriver *sd);

// Data transfer
uint32_t    one_wire_write_bytes(SerialDriver *sd, const uint8_t bytes[], size_t byte_count);
void        one_wire_read_bytes(SerialDriver *sd, uint8_t bytes[], size_t byte_count);


#endif      // _ONE_WIRE_DRIVER_H_
