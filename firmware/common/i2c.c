/*
 * Depot RP2040 Bus Host Firmware - I2C functions
 *
 * @version     1.2.1
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "i2c.h"


/*
 * STATIC PROTOTYPES
 */
static bool check_i2c_pins(uint8_t* data);
static bool pin_check(uint8_t* pins, uint8_t pin);


/*
 * GLOBALS
 */
// FROM 1.1.3
// Access individual boards' pin arrays
extern uint8_t I2C_PIN_PAIRS_BUS_0[];
extern uint8_t I2C_PIN_PAIRS_BUS_1[];


/**
 * @brief Initialise the host's I2C bus.
 *
 * @param its: The I2C state record.
 */
void init_i2c(I2C_State* itr) {

    // Initialise I2C via SDK
    i2c_init(itr->bus, itr->frequency * 1000);

    // Initialise pins
    // The values of SDA_PIN and SCL_PIN are set
    // in the board's individual CMakeLists.txt file.
    gpio_set_function(itr->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(itr->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(itr->sda_pin);
    gpio_pull_up(itr->scl_pin);

    // Mark bus as ready for use
    itr->is_ready = true;

#ifdef DO_UART_DEBUG
    debug_log("I2C activated");
#endif
}


/**
 * @brief Initialise the host's I2C bus.
 *
 * @param its: The I2C state record.
 */
void deinit_i2c(I2C_State* its) {

    // De-initialise I2C via SDK
    i2c_deinit(its->bus);
    its->is_ready = false;
    its->is_started = false;

#ifdef DO_UART_DEBUG
    debug_log("I2C deactivated");
#endif
}


/**
 * @brief Reset the host's I2C bus.
 *
 * @param its: The I2C state record.
 */
void reset_i2c(I2C_State* its) {

    i2c_deinit(its->bus);
    sleep_ms(10);
    i2c_init(its->bus, its->frequency * 1000);

#ifdef DO_UART_DEBUG
    debug_log("I2C reset");
#endif
}


/**
 * @brief Set the frequency of the host's I2C bus.
 *
 * @param its:           The I2C state record.
 * @param frequency_khz: The Frequency in kHz.
 */
void set_i2c_frequency(I2C_State* its, uint32_t frequency_khz) {

    if (frequency_khz != 100 && frequency_khz != 400) return;

    if (its->frequency != frequency_khz) {
        its->frequency = frequency_khz;

#ifdef DO_UART_DEBUG
    debug_log("I2C frequency set: %ikHz", frequency_khz);
#endif
        // If the bus is active, reset it
        if (its->is_ready) {
            reset_i2c(its);
            its->is_started = false;
        }
    }
}


/**
 * @brief Configure the I2C bus: its ID and pins.
 *
 * @param its: The I2C state record.
 * @param data: The received data. Byte 1 is the bus ID,
 *              byte 2 the SDA pin, byte 3 the SCL pin
 *
 * @returns Whether the config was set successfully (`true`) or not (`false`).
 */
bool configure_i2c(I2C_State* its, uint8_t* data) {

#ifdef DO_UART_DEBUG
    debug_log("Switching from pins %i, %i to %i, %i", its->sda_pin, its->scl_pin, data[1], data[2]);
#endif

    // Make sure we have valid data
    if (its->is_ready || !check_i2c_pins(data)) {
        return false;
    }

    // Store the values
    uint8_t bus_index = data[0] & 0x01;
    its->bus = bus_index == 0 ? i2c0 : i2c1;
    its->sda_pin = data[1];
    its->scl_pin = data[2];
    return true;
}


/**
 * @brief Scan the host's I2C bus for devices, and send the results.
 *
 * @param its: The I2C state record.
 */
void send_i2c_scan(I2C_State* its) {

    uint8_t rx_data;
    int reading;
    char scan_buffer[1024] = {0};
    uint32_t device_count = 0;;

    // Generate a list if devices by their addresses.
    // List in the form "13.71.A0."
    for (uint32_t i = 0 ; i < 0x78 ; ++i) {
        reading = i2c_read_timeout_us(its->bus, i, &rx_data, 1, false, 1000);
        if (reading > 0) {
            sprintf(scan_buffer + (device_count * 3), "%02X.", i);
            device_count++;
        }
    }

    // Write 'Z' if there are no devices,
    // or send the device list string
    if (strlen(scan_buffer) == 0) {
        sprintf(scan_buffer, "Z\r\n");
    } else {
        sprintf(scan_buffer + (device_count * 3), "\r\n");
    }

    // Send the scan data back
    tx(scan_buffer, strlen(scan_buffer));
}


/**
 * @brief Scan the host's I2C bus for devices, and send the results.
 *
 * @param its: The I2C state record.
 */
void send_i2c_status(I2C_State* its) {

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
    strncat(model, HW_MODEL, HW_MODEL_NAME_SIZE_MAX);

    // Generate and return the status data string.
    // Data in the form: "1.1.100.110.QTPY-RP2040" or "1.1.100.110.PI-PICO"
    char status_buffer[129] = {0};

    sprintf(status_buffer, "%s.%s.%s.%i.%i.%i.%i.%i.%i.%i.%i.%s.%s\r\n",
            (its->is_ready   ? "1" : "0"),          // 2 chars
            (its->is_started ? "1" : "0"),          // 2 chars
            (its->bus == i2c0 ? "0" : "1"),         // 2 chars
            its->sda_pin,                           // 2-3 chars
            its->scl_pin,                           // 2-3 chars
            its->frequency,                         // 2 chars
            its->address,                           // 2-4 chars
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
 * @brief Check that supplied SDA and SCL pins are valid for the
 *        board we're using
 *
 * @param data: The transmitted pin data.
 *
 * @returns Whether the pins are good (`true`) or not (`false`).
 */
static bool check_i2c_pins(uint8_t* data) {

    i2c_inst_t* bus = (data[0] & 0x01) == 0 ? i2c0 : i2c1;
    uint8_t sda_pin = data[1];
    uint8_t scl_pin = data[2];

    // Same pin? Bail
    if (sda_pin == scl_pin) return false;

    // Select the correct pin-pair array
    uint8_t* pin_pairs = bus == i2c0 ? &I2C_PIN_PAIRS_BUS_0[0] : &I2C_PIN_PAIRS_BUS_1[0];
    if (!pin_check(pin_pairs, sda_pin)) return false;

    pin_pairs = bus == i2c0 ? &I2C_PIN_PAIRS_BUS_0[1] : &I2C_PIN_PAIRS_BUS_1[1];
    if (!pin_check(pin_pairs, scl_pin)) return false;

    if (is_pin_taken(sda_pin) > 0 || is_pin_taken(scl_pin) > 0) return false;
    return true;
}


/**
 * @brief Check that a supplied pin are valid for the
 *        board we're using
 *
 * @param pins: An array of available pins in SDA, SCL pains.
 * @param pin: The pin to check.
 *
 * @returns Whether the pins are good (`true`) or not (`false`).
 */
static bool pin_check(uint8_t* pins, uint8_t pin) {

    uint8_t a_pin = *pins;
    while (a_pin != 255) {
        if (a_pin == pin) return true;
        pins += 2;
        a_pin = *pins;
    }

    return false;
}


/**
 * @brief Check pin usage.
 *
 * @param its: The I2C state record.
 * @param pin: An arbitrary GPIO pin that we're checking.
 *
 * @returns `true` if the pin is in use by the bus, or `false`.
 */
bool is_pin_in_use_by_i2c(I2C_State* its, uint8_t pin) {

    return ((pin == its->sda_pin || pin == its->scl_pin) && its->is_ready);
}
