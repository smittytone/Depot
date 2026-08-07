/*
 * Generic macOS SPI driver
 *
 * Version 1.4.0
 * Copyright © 2026, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _SPI_DRIVER_H
#define _SPI_DRIVER_H


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdint.h>
// App
#include "serialdriver.h"
#include "board_commands.h"


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
    int speed;              // SPI line speed (in kHz)
    int bus_id;
    int mosi;
    int miso;
    int sclk;
    int cs;
    int cpol;
    int cpha;
    int data_size;
} SPIConfigData;


/*
 * PROTOTYPES
 */
// Setup
bool            spi_init(SerialDriver *sd);
bool            spi_deinit(SerialDriver *sd);
bool            spi_set_bus(SerialDriver *sd, SPIConfigData *spid);

// Information
void            spi_get_info(SerialDriver *sd, bool do_print);

// SPI operations
bool            spi_start(SerialDriver *sd, uint8_t op);
bool            spi_stop(SerialDriver *sd);
bool            spi_reset(SerialDriver *sd);

// Data transfer
size_t          spi_write(SerialDriver *sd, const uint8_t bytes[], size_t nn);
void            spi_read(SerialDriver *sd, uint8_t bytes[], size_t nn);



#endif  // _SPI_DRIVER_H
