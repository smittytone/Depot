/*
 *  macOS/Linux Depot GPIO Functions
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "gpio.h"


/**
 * @brief Set a GPIO pin.
 *
 * @param sd:  Pointer to a SerialDriver structure.
 * @param pin: The state, direction and number of the target GPIO.
 *
 * @returns Was the command ACK'd (`true`) or not (`false`).
 */
bool gpio_set_pin(SerialDriver *sd, uint8_t pin) {

    uint8_t set_pin_data[2] = {'g', pin};
    serial_write_to_port(sd->file_descriptor, set_pin_data, 2);
    return serial_ack(sd);
}


/**
 * @brief Read a GPIO pin.
 *
 * @param sd:  Pointer to a SerialDriver structure.
 * @param pin: The state, direction and number of the target GPIO.
 *
 * @returns Was the command ACK'd (`true`) or not (`false`).
 */
uint8_t gpio_get_pin(SerialDriver *sd, uint8_t pin) {

    uint8_t set_pin_data[2] = {'g', pin};
    serial_write_to_port(sd->file_descriptor, set_pin_data, 2);
    uint8_t pin_read = 0;
    size_t result = serial_read_from_port(sd->file_descriptor, &pin_read, 1);
    if (result == -1) print_error("Could not read back from device");
    return pin_read;
}


/**
 * @brief Deinit a GPIO pin.
 *
 * @param sd:  Pointer to a SerialDriver structure.
 * @param pin: The pin number.
 *
 * @returns Was the command ACK'd (`true`) or not (`false`).
 */
bool gpio_clear_pin(SerialDriver *sd, uint8_t pin) {

    uint8_t set_pin_data[3] = {'g', pin, 0xF0};
    serial_write_to_port(sd->file_descriptor, set_pin_data, 3);
    return serial_ack(sd);
}
