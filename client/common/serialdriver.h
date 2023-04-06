/*
 * macOS/Linux Depot Serial Comms Functions
 *
 * Version 1.2.2
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _SERIAL_DRIVER_H
#define _SERIAL_DRIVER_H


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
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <memory.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
// FROM 1.1.2
#include <limits.h>

#ifndef BUILD_FOR_LINUX
#include <IOKit/serial/ioss.h>
#endif

#define __STDC_FORMAT_MACROS



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

// FROM 1.1.2
#define READ_BUS_HOST_TIMEOUT_S         2

// FROM 1.2.0
#define     MODE_CODE_NONE              '0'
#define     MODE_CODE_I2C               'i'
#define     MODE_CODE_SPI               's'
#define     MODE_CODE_UART              'u'
#define     MODE_CODE_ONE_WIRE          'o'


/*
 * STRUCTURES
 */
typedef struct {
    bool            is_connected;       // Set to true when connected
    int             file_descriptor;    // OS file descriptor for host
    char            board_mode;         // Current bus mode
    uint8_t         fw_version_major;
    uint8_t         fw_version_minor;
} SerialDriver;


/*
 * PROTOTYPES
 */
// Serial Port Control Functions
size_t          serial_read_from_port(int fd, uint8_t* b, size_t s);
bool            serial_write_to_port(int fd, const uint8_t* b, size_t s);
void            serial_flush_and_close_port(SerialDriver *sd);

// Board Control Functions
void            serial_connect(SerialDriver *sd, const char* device_path);
bool            serial_set_mode(SerialDriver *sd, char mode_code);

bool            serial_get_last_error(SerialDriver *sd);
bool            serial_set_led(SerialDriver *sd, bool is_on);

size_t          serial_write(SerialDriver *sd, const uint8_t bytes[], size_t byte_count);
void            serial_read(SerialDriver *sd, uint8_t bytes[], size_t byte_count);

bool            serial_ack(SerialDriver *sd);
void            serial_send_command(SerialDriver *sd, char c);


#endif  // _SERIAL_DRIVER_H
