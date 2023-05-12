/*
 * Generic macOS I2C driver
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Global Vars
I2CData i2c;


#pragma mark - I2C Setup Functions

/**
 * @brief Tell the I2C host to Initialise the I2C bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_init(SerialDriver *sd) {

    serial_send_command(sd, 'i');
    return serial_ack(sd);
};


/**
 * @brief Tell the I2C host to de-initialise (Kill) the I2C bus.
 *        FROM 1.1.3
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_deinit(SerialDriver *sd) {

    serial_send_command(sd, 'k');
    return serial_ack(sd);
};


/**
 * @brief Tell the I2C host to set the bus speed.
 *
 * @param sd:    Pointer to a SerialDriver structure.
 * @param speed: Bus frequency in 100kHz. Only 1 or 4 supported.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_set_speed(SerialDriver *sd, long speed) {

    switch(speed) {
        case 1:
            serial_send_command(sd, '1');
            break;
        default:
            serial_send_command(sd, '4');
    }

    return serial_ack(sd);
}


/**
 * @brief Choose the I2C host's target bus: 0 (i2c0) or 1 (i2c1),
 *        and SDA and SCL pins. Firmware will return `ERR` on a
 *        mis-setting.
 *
 * @param sd:      Pointer to a SerialDriver structure.
 * @param bus_id:  The Pico SDK I2C bus ID: 0 or 1.
 * @param sda_pin: The SDA pin GPIO number.
 * @param scl_pin: The SCL pin GPIO number.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_set_bus(SerialDriver *sd, uint8_t bus_id, uint8_t sda_pin, uint8_t scl_pin) {

    if (bus_id < 0 || bus_id > 1) return false;
    uint8_t set_bus_data[4] = {'c', (bus_id & 0x01), sda_pin, scl_pin};
    serial_write_to_port(sd->file_descriptor, set_bus_data, sizeof(set_bus_data));
    return serial_ack(sd);
}


/**
 * @brief Tell the I2C host to reset the I2C bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_reset(SerialDriver *sd) {

    serial_send_command(sd, 'x');
    return serial_ack(sd);
}


#pragma mark - I2C Information Functions

/**
 * @brief Get status info from the USB host.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 * @param do_print: Should we output the results to stderr?
 */
void i2c_get_info(SerialDriver *sd, bool do_print) {

    uint8_t read_buffer[HOST_INFO_BUFFER_MAX_B] = {0};
    serial_send_command(sd, '?');
    size_t result = serial_read_from_port(sd->file_descriptor, read_buffer, 0);
    if (result == -1) {
        print_error("Could not read I2C information from device");
        return;
    }

#ifdef DEBUG
    print_log("Received raw info string: %s", read_buffer);
#endif

    // Data string is, for example,
    // "1.1.100.110.1.1.0.200.A1B23C4D5E6F0A1B.QTPY-RP2040"
    int is_ready = 0;
    int has_started = 0;
    int frequency = 100;
    int address = 0xFF;
    int major = 0;
    int minor = 0;
    int patch = 0;
    int build = 0;
    int bus = 0;
    int sda_pin = -1;
    int scl_pin = -1;
    char string_data[67] = {0};
    char model[25] = {0};
    char pid[17] = {0};

    // Extract the data
    sscanf((char*)read_buffer, "%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%s",
        &is_ready,
        &has_started,
        &bus,
        &sda_pin,
        &scl_pin,
        &frequency,
        &address,
        &major,
        &minor,
        &patch,
        &build,
        string_data
    );

    // Store certain values in the I2C driver record
    // NOTE This involves separately extracting the substrings
    //      from the read `string_data` as sscanf() doesn't
    //      separate them properly
    strncpy(pid, string_data, 16);
    strcpy(model, &string_data[17]);
    i2c.speed = frequency;

    if (do_print) {
        print_log("   I2C host device: %s", model);
        print_log( "  I2C host version: %i.%i.%i (%i)", major, minor, patch, build);
        print_log("       I2C host ID: %s", pid);
        print_log("     Using I2C bus: %s", bus == 0 ? "i2c0" : "i2c1");
        print_log(" I2C bus frequency: %ikHz", frequency);
        print_log(" Pins used for I2C: GP%i (SDA), GP%i (SCL)", sda_pin, scl_pin);
        print_log("    I2C is enabled: %s", is_ready == 1 ? "YES" : "NO");
        print_log("     I2C is active: %s", has_started == 1 ? "YES" : "NO");

        // Check for a 'no device' I2C address
        if (address == 0xFF) {
            print_log("Target I2C address: NONE");
        } else {
            print_log("Target I2C address: 0x%02X", address);
        }

    }
}


/**
 * @brief Scan the I2C bus and list devices.
 *
 * @param sd: Pointer to a SerialDriver structure.
 */
void i2c_scan(SerialDriver *sd) {

    char scan_buffer[SCAN_BUFFER_MAX_B] = {0};
    uint8_t device_list[CONNECTED_DEVICES_MAX_B] = {0};
    uint32_t device_count = 0;

    // Request scan from bus host
    serial_send_command(sd, 'd');
    size_t result = serial_read_from_port(sd->file_descriptor, (uint8_t*)scan_buffer, 0);
    if (result == -1) {
        print_error("Could not read scan data from device");
        return;
    }

    // If we receive Z(ero), there are no connected devices
    if (scan_buffer[0] != 'Z') {
        // Extract device address hex strings and generate
        // integer values. For example:
        // source = "12.71.A0."
        // dest   = [18, 113, 160]

#ifdef DEBUG
        print_log("Buffer: %lu bytes, %lu items", strlen(scan_buffer), strlen(scan_buffer) / 3);
#endif

        for (uint32_t i = 0 ; i < strlen(scan_buffer) ; i += 3) {

            uint8_t value[2] = {0};
            uint32_t count = 0;

            // Get two hex chars and store in 'value'; break on a .
            while(1) {
                uint8_t token = scan_buffer[i + count];
                if (token == 0x2E) break;
                value[count] = token;
                count++;
            }

            device_list[device_count] = (uint8_t)strtol((char *)value, NULL, 16);
            device_count++;
        }
    }

    // Output the device list as a table (even with no devices)

    fprintf(stderr, "   0 1 2 3 4 5 6 7 8 9 A B C D E F");

    for (int i = 0 ; i < 0x80 ; i++) {
        if (i % 16 == 0) fprintf(stderr, "\n%02x ", i);
        if (i < 8 || i > 0x77) {
            fprintf(stderr, "  ");
        } else {
            bool found = false;

            if (device_count > 0) {
                for (int j = 0 ; j < 120 ; j++) {
                    if (device_list[j] == i) {
                        fprintf(stderr, "@ ");
                        found = true;
                        break;
                    }
                }
            }

            if (!found) fprintf(stderr, ". ");
        }
    }

    fprintf(stderr, "\n");
}


#pragma mark - I2C Operation Functions

/**
 * @brief Tell the I2C host to start an I2C transaction.
 *
 * @param sd:      Pointer to a SerialDriver structure.
 * @param address: The target device's I2C address.
 * @param op:      Read (0) or write (1) I2C operation.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool i2c_start(SerialDriver *sd, uint8_t address, uint8_t op) {

    // This is a two-byte command: command + (address | op)
    uint8_t start_data[2] = {'s', ((address << 1) | op)};
    serial_write_to_port(sd->file_descriptor, start_data, sizeof(start_data));
    return serial_ack(sd);
}


/**
 * @brief Tell the I2C host to issue a STOP to the I2C bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 */
bool i2c_stop(SerialDriver *sd) {

    serial_send_command(sd, 'p');
    return serial_ack(sd);
}


#pragma mark - I2C Data Transfer Functions

/**
 * @brief Write data to the I2C host for transmission.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      The bytes to write.
 * @param byte_count: The number of bytes to write.
 *
 * @returns The number of bytes received.
 */
size_t i2c_write(SerialDriver *sd, const uint8_t bytes[], size_t byte_count) {

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
 * @brief Read data from the I2C host.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      A buffer for the bytes to read.
 * @param byte_count: The number of bytes to write.
 */
void i2c_read(SerialDriver *sd, uint8_t bytes[], size_t byte_count) {

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
