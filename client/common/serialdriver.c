/*
 * macOS/Linux Depot Serial Comms Functions
 *
 * Version 1.2.2
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Static Function Prototypes

static int serial_open_port(const char *portname);


#pragma mark - Globals

// Retain the original port settings
struct termios original_settings;


#pragma mark - Serial Port Control Functions

/**
 * @brief Open a serial port.
 *`
 * @param device_path: The target port file, eg. `/dev/cu.usb-modem-10100`
 *
 * @returns The OS file descriptor, or -1 on error.
 */
static int serial_open_port(const char *device_path) {

    struct termios serial_settings;
    speed_t speed = (speed_t)203400;

    // Open the device
    int fd = open(device_path, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        print_error("Could not open the device at %s - %s (%d)", device_path, strerror(errno), errno);
        return fd;
    }

    // Prevent additional opens except by root-owned processes
    if (ioctl(fd, TIOCEXCL) == -1) {
        print_error("Could not set TIOCEXCL on %s - %s (%d)", device_path, strerror(errno), errno);
        goto error;
    }

    // Get the port settings
    if (tcgetattr(fd, &original_settings) != 0) {
        print_error("Could not get the port settings - %s (%d)", strerror(errno), errno);
        goto error;
    }

    serial_settings = original_settings;

    // Calls to read() will return as soon as there is
    // at least one byte available or after 100ms.
    cfmakeraw(&serial_settings);
    serial_settings.c_cc[VMIN]  = 0;
    serial_settings.c_cc[VTIME] = 1;

#ifdef BUILD_FOR_LINUX
    cfsetispeed(&serial_settings, speed);
    cfsetospeed(&serial_settings, speed);
#endif

    if (tcsetattr(fd, TCSANOW, &serial_settings) != 0) {
        print_error("Could not apply the port settings - %s (%d)", strerror(errno), errno);
        goto error;
    }

    // Set the port speed
    // NOTE Needs to go before `tcsetattr()` is called.
    // NOTE For Linux, make sure the speed is standard,
    //      not custom.
#ifndef BUILD_FOR_LINUX
    if (ioctl(fd, IOSSIOSPEED, &speed) == -1) {
        print_error("Could not set port speed to %i bps - %s (%d)", speed, strerror(errno), errno);
        goto error;
    }
#endif

    // Set the latency -- MAY REMOVE IF NOT NEEDED
#ifndef BUILD_FOR_LINUX
    unsigned long lat_us = 1UL;
    if (ioctl(fd, IOSSDATALAT, &lat_us) == -1) {
        print_error("Could not set port latency - %s (%d)", strerror(errno), errno);
        goto error;
    }
#endif

    // Return the File Descriptor
    return fd;

error:
    close(fd);
    return -1;
}

/**
 * @brief Read bytes from the serial port FIFO.
 *
 * @param fd:            The port’s OS file descriptor.
 * @param buffer:        The buffer into which the read data will be written.
 * @param bytes_to_read: The number of bytes to read, or 0 to scan for `\r\n`.
 *
 * @returns The number of bytes read.
 */
size_t serial_read_from_port(int fd, uint8_t* buffer, size_t bytes_to_read) {

    size_t rx_byte_count = 0;
    ssize_t number_read = -1;
    struct timespec now, then;
    clock_gettime(CLOCK_MONOTONIC_RAW, &then);

    if (bytes_to_read == 0) {
        // Unknown number of bytes -- look for \r\n as EOL
        while(1) {
            number_read = read(fd, buffer + rx_byte_count, 1);
            if (number_read == -1) break;
            rx_byte_count += number_read;
            if (rx_byte_count > 2 && buffer[rx_byte_count - 2] == 0x0D && buffer[rx_byte_count - 1] == 0x0A) {
                // Backstep to clear the \r\n from the string
                buffer[rx_byte_count - 2] = '\0';
                buffer[rx_byte_count - 1] = '\0';
                rx_byte_count -= 2;
                break;
            }

            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            if (now.tv_sec - then.tv_sec > READ_BUS_HOST_TIMEOUT_S) {
                print_error("Read timeout: %i bytes read of %i", rx_byte_count, bytes_to_read);
                return -1;
            }
        }
    } else {
        // Read a fixed number of bytes
        while (rx_byte_count < bytes_to_read) {
            // Read in the data a byte at a time
            number_read = read(fd, buffer + rx_byte_count, 1);
            if (number_read != -1) {
                rx_byte_count += number_read;
            }

            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            if (now.tv_sec - then.tv_sec > READ_BUS_HOST_TIMEOUT_S) {
                print_error("Read timeout: %i bytes read of %i", rx_byte_count, bytes_to_read);
                return -1;
            }
        }
    }

#ifdef DEBUG
    // Output the read data for debugging
    fprintf(stderr, "  READ %d of %d: ", (int)rx_byte_count, (int)bytes_to_read);
    for (size_t i = 0 ; i < rx_byte_count ; ++i) {
        fprintf(stderr, "%02X ", 0xFF & buffer[i]);
    }
    fprintf(stderr, "\n");
#endif

    return rx_byte_count;
}


/**
 * @brief Write bytes to the serial port FIFO.
 *
 * @param fd:         The port’s OS file descriptor.
 * @param buffer:     The buffer into which the read data will be written.
 * @param byte_count: The number of bytes to write`.
 *
 * @returns The number of bytes read.
 */
bool serial_write_to_port(int fd, const uint8_t* buffer, size_t byte_count) {

    // Write the bytes
    ssize_t written = write(fd, buffer, byte_count);

#ifdef DEBUG
    // Output the read data for debugging
    if (written < 0) {
        print_log("write() returned an error: %li", written);
        print_log("Data written: %s", buffer);
    } else {
        fprintf(stderr, "WRITE %u: ", (int)byte_count);
        for (int i = 0 ; i < byte_count ; ++i) {
            fprintf(stderr, "%02X ", 0xFF & buffer[i]);
        }

        fprintf(stderr, "\n");

        if (written != byte_count) print_log("write() returned %li", written);
    }
#endif

    return (written == byte_count);
}


/**
 * @brief Flush the port FIFOs and close the port.
 *
 * @param sd: Pointer to a SerialDriver structure.
 */
void serial_flush_and_close_port(SerialDriver *sd) {

    if (sd->file_descriptor != -1) {
        // Drain the FIFOs -- alternative to `tcflush(fd, TCIOFLUSH)`;
        if (tcdrain(sd->file_descriptor) == -1) {
            print_error("Could not flush the port. %s (%d).\n", strerror(errno), errno);
        }

        // Set the port back to how we found it
        if (tcsetattr(sd->file_descriptor, TCSANOW, &original_settings) == -1) {
            print_error("Could not reset port - %s (%d)", strerror(errno), errno);
        }

        // Close the port
        close(sd->file_descriptor);

#ifdef DEBUG
        print_log("Port closed");
#endif
    }
    
    // Mark the connection as broken
    sd->is_connected = false;
}


#pragma mark - Board Driver Functions


/**
 * @brief Connect to the target OneWire host.
 *
 * @param sd:          Pointer to a SerialDriver structure.
 * @param device_path: The device path as a string.
 */
void serial_connect(SerialDriver *sd, const char* device_path) {

    // Mark that we're not connected
    sd->is_connected = false;

    // Open and get the serial port or bail
    sd->file_descriptor = serial_open_port(device_path);
    if (sd->file_descriptor == -1) {
        print_error("Could not open port to device %s", device_path);
        return;
    }

#ifdef DEBUG
    print_log("Device %s FD: %i", device_path, sd->file_descriptor);
#endif

    // Perform a basic communications check
    serial_send_command(sd, '!');
    uint8_t rx[4] = {0};
    size_t result = serial_read_from_port(sd->file_descriptor, rx, 4);

    if (rx[2] != '\r') {
        sd->fw_version_major = rx[2];
        sd->fw_version_minor = rx[3];
    } else {
        sd->fw_version_major = 1;
        sd->fw_version_minor = 1;
    }

    if (result == -1 || ((rx[0] != 'O') && (rx[1] != 'K'))) {
        print_error("No response from device %s", device_path);
        return;
    }

    // Got this far? We're good to go
    sd->is_connected = true;
}


/**
 * @brief Set board mode.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns The op was ACK'd (1) or not (0).
 */
bool serial_ack(SerialDriver *sd) {

    uint8_t read_buffer[1] = {0};
    if (serial_read_from_port(sd->file_descriptor, read_buffer, 1) != 1) return false;
    bool ackd = ((read_buffer[0] & ACK) == ACK);

#ifdef DEBUG
    print_log((ackd ? "ACK" : "ERR"));
#endif

    return ackd;
}


/**
 * @brief Set the board's current bus mode.
 *
 * @param sd:        Pointer to a SerialDriver structure.
 * @param mode_code: Char code for the desired mode.
 *
 * @returns The op was ACK'd (1) or not (0).
 */
bool serial_set_mode(SerialDriver *sd, char mode_code) {

    char cmd[2] = "#i";
    cmd[1] = mode_code;
    serial_write_to_port(sd->file_descriptor, (uint8_t*)&cmd, 2);

    bool success = serial_ack(sd);
    if (success) sd->board_mode = mode_code;
    return success;
}


/**
 * @brief Control the board's LED heartbeat.
 *
 * @param sd:    Pointer to a SerialDriver structure.
 * @param is_on: Whether the LED should by on (`true`) or off (`false`).
 *
 * @returns Was the command ACK'd (`true`) or not (`false`).
 */
bool serial_set_led(SerialDriver *sd, bool is_on) {

    uint8_t set_led_data[2] = {'*', (is_on ? 1 : 0)};
    serial_write_to_port(sd->file_descriptor, set_led_data, sizeof(set_led_data));
    return serial_ack(sd);
}


/**
 * @brief Control the board's LED heartbeat.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Was the command ACK'd (`true`) or not (`false`).
 */
bool serial_get_last_error(SerialDriver *sd) {

    uint8_t last_error;

    serial_send_command(sd, '$');
    size_t result = serial_read_from_port(sd->file_descriptor, &last_error, 1);
    if (result == -1) {
        print_error("Could not read last error from device");
        return false;
    }

    // Check for old firmware versions: they'll ERR
    if (last_error == ERR) {
        print_warning("Board is on firmware pre-1.1.3 and doesn't support this feature");
    } else {
        print_log("Last error code recorded by board: 0x%02X", last_error);

    }

    return true;
}


/**
 * @brief Write a single-byte command to the serial port.
 *
 * @param sd: Pointer to a SerialDriver structure.
 * @param c:  A command character.
 */
void serial_send_command(SerialDriver* sd, char c) {

    serial_write_to_port(sd->file_descriptor, (uint8_t*)&c, 1);
}


/**
 * @brief Write data to the board for transmission.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      The bytes to write.
 * @param byte_count: The number of bytes to write.
 *
 * @returns The number of bytes received.
 */
size_t serial_write(SerialDriver *sd, const uint8_t bytes[], size_t byte_count) {

    // Count the bytes sent
    int count = 0;
    bool ack = false;

    // Write the data out in blocks of 64 bytes
    for (size_t i = 0 ; i < byte_count ; i += 64) {
        // Calculate the data length for the prefix byte
        size_t length = ((byte_count - i) < 64) ? (byte_count - i) : 64;
        uint8_t write_cmd[65] = {(uint8_t)(PREFIX_BYTE_WRITE + length - 1)};

        // Write a block of bytes to the send buffer
        memcpy(write_cmd + 1, bytes + i, length);

        // Write out the block -- use ACK as byte count
        serial_write_to_port(sd->file_descriptor, write_cmd, 1 + length);
        ack = serial_ack(sd);
        if (!ack) break;
        count += length;
    }

    return count;
}


/**
 * @brief Read data from the board.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      A buffer for the bytes to read.
 * @param byte_count: The number of bytes to write.
 */
void serial_read(SerialDriver *sd, uint8_t bytes[], size_t byte_count) {

    for (size_t i = 0 ; i < byte_count ; i += 64) {
        // Calculate data length for prefix byte
        size_t length = ((byte_count - i) < 64) ? (byte_count - i) : 64;
        uint8_t read_cmd[1] = {(uint8_t)(PREFIX_BYTE_READ + length - 1)};

        serial_write_to_port(sd->file_descriptor, read_cmd, 1);
        size_t result = serial_read_from_port(sd->file_descriptor, bytes + i, length);
        if (result == -1) {
            print_error("Could not read back from device");
        } else {
            for (size_t i = 0 ; i < result ; ++i) {
                fprintf(stdout, "%02X", bytes[i]);
            }

            fprintf(stdout, "\n");
        }
    }
}
