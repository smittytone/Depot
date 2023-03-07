/*
 * Depot RP2040 Bus Host Firmware - 1-Wire functions
 *
 * @version     1.2.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "onewire.h"

// https://www.analog.com/en/technical-articles/1wire-communication-through-software.html


/*
 * STATIC PROTOTYPES
 */
static void     ow_bit_out(OneWireState* ows, uint8_t bit_value);
static uint8_t  ow_bit_in(OneWireState* ows);
static uint32_t ow_search(OneWireState* ows, uint32_t next_node, int64_t* cid);
static void     ow_discover_devices(OneWireState* ows);


/**
 * @brief Reset and test the bus; if it's good, enumerate the devices on the bus.
 */
void ow_init(OneWireState* ows) {

    // Clear device store
    memset(ows->device_ids, 0x00, 64 << 3);
    ows->device_count = 0;
    ows->current_device = 0;

    // Reset the bus and enumerate devices
    if (ow_reset(ows)) ow_discover_devices(ows);

    // Save bus state
    ows->is_ready = (ows->device_count != 0);
}


/**
 * @brief Reset the OneWire bus.
 *
 * @param ows: Pointer to a OneWireState structure.
 *
 * @returns `true` if devices are present on the bus, otherwise `false`.
 */
bool ow_reset(OneWireState* ows) {

    // Hold period
    sleep_us(DELAY_STANDARD_G_US);

    // Drive LO
    gpio_init(ows->data_pin);
    gpio_set_dir(ows->data_pin, GPIO_OUT);
    gpio_put(ows->data_pin, false);

    // Hold period
    sleep_us(DELAY_STANDARD_H_US);

    // Float HI
    gpio_set_dir(ows->data_pin, GPIO_IN);

    // Hold period
    sleep_us(DELAY_STANDARD_I_US);

    // Sample the line for devices
    bool devices_present = !gpio_get(ows->data_pin);

    // Post-sample period
    sleep_us(DELAY_STANDARD_J_US);

    return devices_present;
}


/**
 * @brief Enumerate devices on the bus.
 *
 * @param ows: Pointer to a OneWireState structure.
 *
 * @returns The number of discovered devices.
 */
static void ow_discover_devices(OneWireState* ows) {

#ifdef DO_UART_DEBUG
    debug_log("Discovering...");;
#endif

    // Clear device store
    memset(ows->device_ids, 0x00, 64 * 8);
    ows->current_device = 0;

    uint64_t current_id = 0;
    uint32_t device_count = 0;

    // Begin the enumeration at address 65
    uint32_t next_device = 65;

    // This is the pointer to the device in the devices space
    while (next_device > 0) {
        next_device = ow_search(ows, next_device, &current_id);
        ows->device_ids[device_count] = current_id;
        device_count++;
    }

    ows->device_count = device_count;
}


/**
 * @brief Set the 1-Wire data pin.
 *
 * @param ows: Pointer to a OneWireState structure.
 * @param pin: The selected pin number.
 *
 * @returns `true` if the configuration is accepted, otherwise `false`.
 */
bool ow_configure(OneWireState* ows, uint32_t pin) {

    // Is the pin already in use?
    if (is_pin_taken(pin) > 0) return false;

    // Is the bus in use?
    if (ows->is_ready) return false;

    ows->data_pin = pin;
    return true;
}


/**
 * @brief Write out a single bit.
 *
 * @param ows:       Pointer to a OneWireState structure.
 * @param bit_value: The bit, 1 or 0.
 */
static void ow_bit_out(OneWireState* ows, uint8_t bit_value) {

    /*
        All data and commands are transmitted least significant bit first over the 1-Wire bus.
    */

    // Drive LO
    gpio_set_dir(ows->data_pin, GPIO_OUT);
    gpio_put(ows->data_pin, false);

    // Line hold period
    bit_value &= 0x01;
    sleep_us(bit_value == BIT_VALUE_1 ? DELAY_STANDARD_A_US : DELAY_STANDARD_C_US);;

    // Float HI
    gpio_set_dir(ows->data_pin, GPIO_IN);

    // Wait period
    sleep_us(bit_value == BIT_VALUE_1 ? DELAY_STANDARD_B_US : DELAY_STANDARD_D_US);;

    // Recovery period (min)
    sleep_us(DELAY_STANDARD_R_US);
}


/**
 * @brief Write out a byte.
 *
 * @param ows:        Pointer to a OneWireState structure.
 * @param byte_value: The byte to send.
 */
void ow_write_byte(OneWireState* ows, uint8_t byte_value) {

    for (uint8_t i = 0 ; i < 8 ; ++i, byte_value >>= 1) {
        ow_bit_out(ows, byte_value & 0x01);
    }
}


/**
 * @brief Read in a single bit.
 *
 * @param ows: Pointer to a OneWireState structure.
 *
 * @returns The value of the bit (`1` or `0`).
 */
static uint8_t ow_bit_in(OneWireState* ows) {

    // Drive LO
    gpio_set_dir(ows->data_pin, GPIO_OUT);
    gpio_put(ows->data_pin, false);

    // Hold period
    sleep_us(DELAY_STANDARD_A_US);

    // Float HI
    gpio_set_dir(ows->data_pin, GPIO_IN);

    // Pre-sample period
    sleep_us(DELAY_STANDARD_E_US);

    // Sample value
    uint8_t sample = gpio_get(ows->data_pin) ? BIT_VALUE_1 : BIT_VALUE_0;

    // Post-sample period
    sleep_us(DELAY_STANDARD_F_US);

    // Recovery period (min)
    sleep_us(DELAY_STANDARD_R_US);

    return sample;
}


/**
 * @brief Read in a byte.
 *
 * @param ows: Pointer to a OneWireState structure.
 *
 * @returns The value of the byte.
 */
uint8_t ow_read_byte(OneWireState* ows) {

    uint8_t value = 0;
    for (uint32_t i = 0 ; i < 8 ; ++i) {
        value >>= 1;
        if (ow_bit_in(ows) == BIT_VALUE_1) value |= 0x80;
    }

    return (value & 0xFF);
}


/**
 * @brief Device enumeration support function. Progresses one step up the tree
 *        from the current device, returning the next current device along.
 *        Called by `ow_discover_devices()`
 *
 * @param ows:       Pointer to a OneWireState structure.
 * @param next_node: The next search point.
 * @param cid:       Pointer to the caller's current ID store.
 *
 * @returns The next node. The value `0` indicates all devices have been found.
 */
static uint32_t ow_search(OneWireState* ows, uint32_t next_node, int64_t* cid) {

    uint32_t last_fork_point = 0;

    // Reset the bus but exit if no device found
    if (ow_reset(ows)) {
        // If there are 1-Wire device(s) on the bus - for which ow_reset() checks -
        // this function readies them by issuing the 1-Wire SEARCH command (0xF0)
        ow_write_byte(ows, OW_CMD_SEARCH_ROM);

        // Work along the 64-bit ROM code, bit by bit, from LSB to MSB
        for (uint32_t i = 64 ; i > 0 ; i--) {
            uint8_t byte = (i - 1) >> 3;

            // Read bit from bus
            uint8_t first_bit = ow_bit_in(ows);

            // Read the next bit, the first's complement
            if (ow_bit_in(ows) == BIT_VALUE_1) {
                if (first_bit == BIT_VALUE_1) {
                    // If first bit is 1 too, this indicates no further devices
                    // so put pointer back to the start and break out of the loop
                    last_fork_point = 0;
                    break;
                }
            } else if (first_bit == BIT_VALUE_0) {
                // First and second bits are both 0
                uint8_t id_byte = ((*cid >> (byte << 3)) & 0xFF);
                if (next_node > i || ((next_node != i) && (id_byte & 1))) {
                    // Take the '1' direction on this point
                    first_bit = BIT_VALUE_1;
                    last_fork_point = i;
                }
            }

            // Write the 'direction' bit. If it's, say, 1, then all further
            // devices with a 0 at the current ID bit location will go offline
            ow_bit_out(ows, first_bit);

            // Shift out the previous path bits, add on the msb side the new choosen path bit
            // current_id[byte] = (current_id[byte] >> 1) + 0x80 * bit;
            uint64_t a = *cid;
            a >>= 1;
            a |= (first_bit == BIT_VALUE_1 ? 0x8000000000000000 : 0x00);
            *cid = a;
        }

#ifdef DO_UART_DEBUG
    debug_log("Device found: %016llX", *cid);
#endif

    }

    // Return the last fork point for next search
    return last_fork_point;
}


/**
 * @brief Send device information.
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void ow_send_state(OneWireState* ows) {

    // Get the RP2040 unique ID
    char pid[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
    pico_get_unique_board_id_string(pid, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
    // eg. DF6050788B3E1A2E

    // Get the firmware version as integers
    int major, minor, patch;
    sscanf(FW_VERSION, "%i.%i.%i",
        &major,
        &minor,
        &patch
    );

    char model[HW_MODEL_NAME_SIZE_MAX + 1] = {0};
    //strncpy(model, model, HW_MODEL_NAME_SIZE_MAX);
    strncat(model, HW_MODEL, HW_MODEL_NAME_SIZE_MAX);

    // Generate and return the status data string.
    // Data in the form: "1.1.100.110.QTPY-RP2040" or "1.1.100.110.PI-PICO"
    char status_buffer[129] = {0};
    sprintf(status_buffer, "%s.%i.%i.%i.%i.%i.%i.%s.%s\r\n",
            (ows->is_ready   ? "1" : "0"),          // 2 chars
            ows->data_pin,                          // 2-3 chars
            ows->device_count,                      // 2-3 chars
            major,                                  // 2-4 chars
            minor,                                  // 2-4 chars
            patch,                                  // 2-4 chars
            BUILD_NUM,                              // 2-4 chars
            pid,                                    // 17 chars
            model);                                 // 2-17 chars
                                                    // == 41-68 chars

    // Send the data
    tx(status_buffer, strlen(status_buffer));
}


/**
 * @brief Send device information.
 *
 * @param ows: Pointer to a OneWireState structure.
 */
void ow_send_scan(OneWireState* ows) {

    char scan_buffer[1025] = {0};

    // Bus not yet primed? Do so now
    if (!ows->is_ready) ow_init(ows);

    // Write 'Z' if there are no devices,
    // or send the device list string
    if (ows->device_count == 0) {
        sprintf(scan_buffer, "Z\r\n");
    } else {
        // The string comprises 16 bytes per device: eight hex pairs
        // for the device’s eight bytes of ID.
        for (uint32_t i = 0 ; i < ows->device_count ; ++i) {
            sprintf(scan_buffer + (i << 4), "%016llX", ows->device_ids[i]);
        }

        sprintf(scan_buffer + (ows->device_count << 4), "\r\n");
    }

    // Send the scan data back
    tx(scan_buffer, strlen(scan_buffer));
}


/**
 * @brief Check pin usage.
 *
 * @param ows: Pointer to a OneWireState structure.
 * @param pin: An arbitrary GPIO pin that we're checking.
 *
 * @returns `true` if the pin is in use by the bus, or `false`.
 */
bool is_pin_in_use_by_ow(OneWireState* ows, uint8_t pin) {

    return (pin == ows->data_pin && ows->is_ready);
}
