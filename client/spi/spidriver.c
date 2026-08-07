/*
 * Generic macOS SPI driver
 *
 * Version 1.4.0
 * Copyright © 2026, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#pragma mark - Includes

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
// App
#include "spidriver.h"
#include "utils.h"

#pragma mark - Global Vars

SPIConfigData spi;


#pragma mark - SPI Setup Functions

/**
 * @brief Tell the SPI host to Initialise the SPI bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool spi_init(SerialDriver *sd) {

    serial_send_command(sd, CMD_MULTIBUS_INIT_BUS);
    return serial_ack(sd);
};


/**
 * @brief Tell the SPI host to de-initialise (Kill) the SPI bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool spi_deinit(SerialDriver *sd) {

    serial_send_command(sd, CMD_MULTIBUS_DEINIT_BUS);
    return serial_ack(sd);
};


/**
 * @brief Choose the SPI host's target bus: 0 (`spi0`) or 1 (`spi1`),
 *        and MOSI, MISO, SCLK and CS pins.
 *        Firmware will return `ERR` on a mis-setting.
 *
 * @param sd:   Pointer to a SerialDriver structure.
 * @param spid: Pointer to a SPIConfigData config structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool spi_set_bus(SerialDriver *sd, SPIConfigData *spid) {

    if (spid->bus_id < 0 || spid->bus_id > 1) return false;
    uint8_t set_bus_data[9] = {CMD_MULTIBUS_CONFIGURE_BUS, (spid->bus_id & 0x01), spid->mosi, spid->miso, spid->sclk, spid->cs, spid->cpol, spid->cpha, spid->data_size};
    serial_write_to_port(sd->file_descriptor, set_bus_data, sizeof(set_bus_data));
    return serial_ack(sd);
}


/**
 * @brief Tell the SPI host to reset the SPI bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool spi_reset(SerialDriver *sd) {

    serial_send_command(sd, CMD_MULTIBUS_RESET_BUS);
    return serial_ack(sd);
}


#pragma mark - SPI Information Functions

/**
 * @brief Get status info from the USB host.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 * @param do_print: Should we output the results to stderr?
 */
void spi_get_info(SerialDriver *sd, bool do_print) {

    uint8_t read_buffer[HOST_INFO_BUFFER_MAX_B] = {0};
    serial_send_command(sd, CMD_GET_STATUS);
    size_t result = serial_read_from_port(sd->file_descriptor, read_buffer, 0);
    if (result == -1) {
        print_error("Could not read SPI information from device");
        return;
    }

#ifdef DEBUG
    print_log("Received raw info string: %s", read_buffer);
#endif

    // Data string is, for example,
    // "1.1.100.110.1.1.0.200.0.2.3.4.5.x.A1B23C4D5E6F0A1B.QTPY-RP2040"
    int is_ready = 0;
    int has_started = 0;
    int bus = 0;
    int frequency = 100;
    int major = 0;
    int minor = 0;
    int patch = 0;
    int build = 0;
    int mosi_pin = -1;
    int miso_pin = -1;
    int sclk_pin = -1;
    int cs_pin = -1;
    int cpol = -1;
    int cpha = -1;
    int data_size = -1;
    char string_data[67] = {0};
    char model[25] = {0};
    char pid[17] = {0};

    // Extract the data
    sscanf((char*)read_buffer, "%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%s",
           &is_ready,
           &has_started,
           &bus,
           &mosi_pin,
           &miso_pin,
           &sclk_pin,
           &cs_pin,
           &frequency,
           &data_size,
           &cpol,
           &cpha,
           &major,
           &minor,
           &patch,
           &build,
           string_data
    );

    // Store certain values in the SPI driver record
    // NOTE This involves separately extracting the substrings
    //      from the read `string_data` as sscanf() doesn't
    //      separate them properly
    print_log("**** %s", string_data);
    strncpy(pid, string_data, 16);
    strncpy(model, &string_data[17], 24);
    spi.speed = frequency;

    if (do_print) {
        print_log("   SPI host device: %s", model);
        print_log( "  SPI host version: %i.%i.%i (%i)", major, minor, patch, build);
        print_log("       SPI host ID: %s", pid);
        print_log("     Using SPI bus: %s", bus == 0 ? "spi0" : "spi1");
        print_log(" SPI bus frequency: %ikHz", frequency);
        print_log(" Pins used for SPI: GP%i (MOSI), GP%i (MISO), GP%i (SCLK), GP%i (CS)", mosi_pin, miso_pin, sclk_pin, cs_pin);
        print_log("        SPI format: CPOL %s, CPHA %s", (cpol == 1 ? "enabled" : "disabled"), (cpha == 1 ? "enabled" : "disabled"));
        print_log("    SPI is enabled: %s", is_ready == 1 ? "YES" : "NO");
        print_log("     SPI is active: %s", has_started == 1 ? "YES" : "NO");
    }
}


#pragma mark - SPI Operation Functions

/**
 * @brief Tell the SPI host to start a SPI transaction.
 *
 * @param sd:      Pointer to a SerialDriver structure.
 * @param op:      Read (0) or write (1) SPI operation.
 *
 * @returns Whether the command was ACK'd (`true`) or not (`false`).
 */
bool spi_start(SerialDriver *sd, uint8_t op) {

    // This is a two-byte command: command + op)
    uint8_t start_data[2] = {CMD_MULTIBUS_START, op};
    serial_write_to_port(sd->file_descriptor, start_data, sizeof(start_data));
    return serial_ack(sd);
}


/**
 * @brief Tell the SPI host to halt the SPI bus.
 *
 * @param sd: Pointer to a SerialDriver structure.
 */
bool spi_stop(SerialDriver *sd) {

    serial_send_command(sd, CMD_MULTIBUS_STOP);
    return serial_ack(sd);
}


#pragma mark - SPI Data Transfer Functions

/**
 * @brief Write data to the I2SPIC host for transmission.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      The bytes to write.
 * @param byte_count: The number of bytes to write.
 *
 * @returns The number of bytes received.
 */
size_t spi_write(SerialDriver *sd, const uint8_t bytes[], size_t byte_count) {

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
 * @brief Read data from the SPI host.
 *
 * @param sd:         Pointer to a SerialDriver structure.
 * @param bytes:      A buffer for the bytes to read.
 * @param byte_count: The number of bytes to write.
 */
void spi_read(SerialDriver *sd, uint8_t bytes[], size_t byte_count) {

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
