/*
 * macOS/Linux Depot 1-Wire driver
 *
 * Version 1.2.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "owdriver.h"


#pragma mark -  1-Wire Setup Functions

/**
 * @brief Tell the board to Initialise 1-Wire.
 *        This includes in implicit reset.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool one_wire_init(SerialDriver *sd) {

    serial_send_command(sd, 'i');
    return serial_ack(sd);
}


/**
 * @brief Tell the board to reset 1-Wire.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool one_wire_reset(SerialDriver *sd) {

    serial_send_command(sd, 'x');
    return serial_ack(sd);
}


/**
 * @brief Choose the 1-Wire bus’ data pin.
 *        Firmware will return `ERR` on a mis-setting.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 * @param data_pin: The data pin GPIO number.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool one_wire_configure_bus(SerialDriver *sd, uint8_t data_pin) {

    uint8_t set_bus_data[2] = {'c', data_pin};
    serial_write_to_port(sd->file_descriptor, set_bus_data, 2);
    return serial_ack(sd);
}


#pragma mark -  1-Wire Information Functions

/**
 * @brief Request 1-Wire information from the board.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 * @param do_print: Should the data be output?
 */
void one_wire_get_info(SerialDriver *sd, bool do_print) {

    uint8_t read_buffer[HOST_INFO_BUFFER_MAX_B] = {0};
    serial_send_command(sd, '?');
    size_t result = serial_read_from_port(sd->file_descriptor, read_buffer, 0);
    if (result == -1) {
        print_error("Could not read OnwWire information from device");
        return;
    }

#ifdef DEBUG
    print_log("Received raw info string: %s", read_buffer);
#endif

    // Data string is, for example,
    // "1.1.100.110.1.1.0.200.A1B23C4D5E6F0A1B.QTPY-RP2040"
    int is_ready = 0;
    int major = 0;
    int minor = 0;
    int patch = 0;
    int build = 0;
    int data_pin = -1;
    int device_count = 0;
    char string_data[67] = {0};
    char model[25] = {0};
    char pid[17] = {0};

    // Extract the data
    sscanf((char*)read_buffer, "%i.%i.%i.%i.%i.%i.%i.%s",
        &is_ready,
        &data_pin,
        &device_count,
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

    if (do_print) {
        print_log(" 1-Wire host device: %s", model);
        print_log("1-Wire host version: %i.%i.%i (%i)", major, minor, patch, build);
        print_log("     1-Wire host ID: %s", pid);
        print_log("    1-Wire data pin: GP%i", data_pin);
        print_log("  1-Wire is enabled: %s", is_ready == 1 ? "YES" : "NO");
        print_log("     1-Wire devices: %i", device_count);
    }
}


/**
 * @brief Request a 1-Wire device scan.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 */
void one_wire_scan(SerialDriver *sd) {

    char scan_buffer[SCAN_BUFFER_MAX_B] = {0};
    uint64_t device_list[63] = {0};
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
        // Extract device IDs from a sequence of 16 bytes per ID:
        // 16 bytes = hex string representation of 64-bit value

#ifdef DEBUG
        print_log("Buffer: %lu bytes, %lu items", strlen(scan_buffer), strlen(scan_buffer) >> 4);
        print_log("Buffer: %s", scan_buffer);
#endif

        for (uint32_t i = 0 ; i < strlen(scan_buffer) ; i += 16) {
            char value[17] = {0};
            strncpy(value, scan_buffer + i, 16);
            device_list[device_count] = strtoull((char *)value, NULL, 16);
            device_count++;
        }

        for (int i = 0 ; i < device_count ; i++) {
            uint8_t fid = (device_list[i] & 0xFF);
            uint64_t sid = (device_list[i] & 0xFFFFFFFFFFFF00) >> 8;
            fprintf(stderr, "%02i. Family ID: %02X, Serial: %012llX\n", i + 1, fid, sid);
        }
    } else {
        fprintf(stderr, "No 1-Wire devices present\n");
    }
}


#pragma mark -  1-Wire Data Transfer Functions

/**
 * @brief Read data from OnwWire.
 *
 * @param sd:         Pointer to an SerialDriver structure.
 * @param bytes:      A buffer for the bytes to read.
 * @param byte_count: The number of bytes to write.
 */
void one_wire_read_bytes(SerialDriver *sd, uint8_t bytes[], size_t byte_count) {

    for (size_t i = 0 ; i < byte_count ; i += 64) {
        // Calculate data length for prefix byte
        size_t length = ((byte_count - i) < 64) ? (byte_count - i) : 64;
        uint8_t read_cmd[1] = {(uint8_t)(PREFIX_BYTE_READ + length - 1)};
        serial_write_to_port(sd->file_descriptor, read_cmd, 1);
        size_t result = serial_read_from_port(sd->file_descriptor, bytes, length);
        if (result == -1) {
            print_error("Could not read back from device");
        } else {
#ifndef SWIFT_BUILD
            for (size_t i = 0 ; i < result ; ++i) {
                fprintf(stdout, "%02X", bytes[i]);
            }

            fprintf(stdout, "\n");
#endif
        }
    }
}


/**
 * @brief Write data to the board for 1-Wire transmission.
 *
 * @param sd:         Pointer to an SerialDriver structure.
 * @param bytes:      The bytes to write.
 * @param byte_count: The number of bytes to write.
 *
 * @returns The number of bytes received.
 */
uint32_t one_wire_write_bytes(SerialDriver *sd, const uint8_t bytes[], size_t byte_count) {

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


#pragma mark -  1-Wire Convenience Functions

/**
 * @brief Issue command to ignore device ID(s).
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void one_wire_cmd_skip_rom(SerialDriver *sd) {

    uint8_t cmd = OW_CMD_SKIP_ROM;
    one_wire_write_bytes(sd, &cmd, 1);
}


/**
 * @brief Issue command to read a device’s ID.
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void one_wire_cmd_read_rom(SerialDriver *sd) {

    uint8_t cmd = OW_CMD_READ_ROM;
    one_wire_write_bytes(sd, &cmd, 1);
}

/**
 * @brief Issue command to begin enumerating IDs.
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void one_wire_cmd_search_rom(SerialDriver *sd) {

    uint8_t cmd = OW_CMD_SEARCH_ROM;
    one_wire_write_bytes(sd, &cmd, 1);
}

/**
 * @brief Issue command to select a device with a specific ID.
 *        The next 64 bits to be written will be the known ID.
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void one_wire_cmd_match_rom(SerialDriver *sd) {

    uint8_t cmd = OW_CMD_MATCH_ROM;
    one_wire_write_bytes(sd, &cmd, 1);
}
