/*
 * Depot RP2040 Bus Host Firmware - SPI functions
 *
 * @version     1.4.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
// C
#include <string.h>
#include <stdio.h>
// Pico
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
// Depot
#include "serial.h"
#include "spi.h"


/*
 * STATIC PROTOTYPES
 */
// FROM 1.4.0
static bool check_spi_pins(uint8_t* data);
static bool spi_pin_check(uint8_t* pins, uint8_t offset, uint8_t pin);


/*
 * GLOBALS
 */
// FROM 1.4.0
// Access individual boards' pin arrays
// (see, for example, `firmware/pico/pins.c`)
extern uint8_t SPI_PIN_TRIPLES_BUS_0[];
extern uint8_t SPI_PIN_TRIPLES_BUS_1[];


/**
 * @brief Initialise the host's SPI bus.
 *
 * @param sps: The SPI state record.
 */
void init_spi(SPI_State* sps) {

    // Initialise SPI via SDK
    spi_init(sps->bus, sps->frequency);
    spi_set_format (sps->bus, sps->data_size, sps->cpol, sps->cpha, SPI_MSB_FIRST);

    // Initialise the clock, MOSI (TX) and MISO (RX) pins
    // The values are set either by the board's own CMakeLists.txt
    // defaults, or via a `configure_spi()` call from the host
    gpio_set_function(sps->sclk_pin, GPIO_FUNC_SPI);
    gpio_set_function(sps->mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(sps->miso_pin, GPIO_FUNC_SPI);

    // The CS pin is toggled manually (it's active low) rather than
    // handed to the SPI peripheral, so the host can drive an
    // arbitrary GPIO and isn't tied to the RP2040's own CSn pins
    gpio_init(sps->cs_pin);
    gpio_set_dir(sps->cs_pin, GPIO_OUT);
    gpio_put(sps->cs_pin, true);

    // Mark bus as ready for use
    sps->is_ready = true;

#ifdef DO_UART_DEBUG
    debug_log("SPI activated");
#endif
}


/**
 * @brief De-initialise the host's SPI bus.
 *
 * @param sps: The SPI state record.
 */
void deinit_spi(SPI_State* sps) {

    // De-initialise SPI via SDK
    spi_deinit(sps->bus);
    sps->is_ready = false;
    sps->is_started = false;

#ifdef DO_UART_DEBUG
    debug_log("SPI deactivated");
#endif
}


/**
 * @brief Reset the host's SPI bus.
 *
 * @param sps: The SPI state record.
 */
void reset_spi(SPI_State* sps) {

    spi_deinit(sps->bus);
    sleep_ms(10);
    init_spi(sps);
    //spi_init(sps->bus, sps->frequency);

#ifdef DO_UART_DEBUG
    debug_log("SPI reset");
#endif
}


/**
 * @brief Set the frequency of the host's SPI bus.
 *
 * @param sps:           The SPI state record.
 * @param frequency_khz: The frequency in kHz.
 */
void set_spi_frequency(SPI_State* sps, uint32_t frequency_khz) {

    if (frequency_khz == 0) return;

    uint32_t frequency_hz = frequency_khz * 1000;
    if (sps->frequency != frequency_hz) {
        sps->frequency = frequency_hz;

#ifdef DO_UART_DEBUG
        debug_log("SPI frequency set: %ikHz", frequency_khz);
#endif

        // If the bus is active, reset it
        if (sps->is_ready) {
            reset_spi(sps);
            sps->is_started = false;
        }
    }
}


/**
 * @brief Configure the SPI bus: its ID and pins.
 *
 * @param sps:  The SPI state record.
 * @param data: The received data. Byte 0 is the bus ID (bit 0 only),
 *              byte 1 the SCLK pin, byte 2 the MOSI (TX) pin,
 *              byte 3 the MISO (RX) pin, byte 4 the CS pin,
 *              byte 4 the CPOL value, byte 5 the CPHA value and
 *              byte 6 the data size.
 *
 * @returns Whether the config was set successfully (`true`) or not (`false`).
 */
bool configure_spi(SPI_State* sps, uint8_t* data) {

#ifdef DO_UART_DEBUG
    debug_log("Switching from pins %i, %i, %i, %i to %i, %i, %i, %i",
        sps->sclk_pin, sps->mosi_pin, sps->miso_pin, sps->cs_pin,
        data[SPI_CONFIG_BYTE_SCLK_pin], data[SPI_CONFIG_BYTE_MOSI_PIN],
        data[SPI_CONFIG_BYTE_MISO_PIN], data[SPI_CONFIG_BYTE_CS_PIN]);
#endif

    // Make sure we have valid data, and that we're not already up and running
    if (sps->is_ready || !check_spi_pins(data)) {
        return false;
    }

    // Store the values
    uint8_t bus_index = data[0] & 0x01;
    sps->bus = bus_index == 0 ? spi0 : spi1;
    sps->mosi_pin  = data[SPI_CONFIG_BYTE_MOSI_PIN];
    sps->miso_pin  = data[SPI_CONFIG_BYTE_MISO_PIN];
    sps->sclk_pin  = data[SPI_CONFIG_BYTE_SCLK_PIN];
    sps->cs_pin    = data[SPI_CONFIG_BYTE_CS_PIN];
    sps->cpol      = data[SPI_CONFIG_BYTE_CPOL] & 0x01;
    sps->cpha      = data[SPI_CONFIG_BYTE_CPHA] & 0x01;
    sps->data_size = data[SPI_CONFIG_BYTE_DATA_SIZE] & 0x0F;
    return true;
}


/**
 * @brief Check that supplied SCK, MOSI, MISO and CS pins are valid for the
 *        board we're using.
 *
 * @param data: The transmitted pin data.
 *
 * @returns Whether the pins are good (`true`) or not (`false`).
 */
// FROM 1.4.0
static bool check_spi_pins(uint8_t* data) {

    uint8_t bus_index = data[0] & 0x01;
    uint8_t sclk_pin = data[SPI_CONFIG_BYTE_SCLK_PIN];
    uint8_t mosi_pin = data[SPI_CONFIG_BYTE_MOSI_PIN];
    uint8_t miso_pin = data[SPI_CONFIG_BYTE_MISO_PIN];
    uint8_t cs_pin   = data[SPI_CONFIG_BYTE_CS_PIN];

    // No two pins may be the same
    if (sclk_pin == mosi_pin || sclk_pin == miso_pin || sclk_pin == cs_pin ||
        mosi_pin == miso_pin || mosi_pin == cs_pin || miso_pin == cs_pin) {
        return false;
    }

    // Select the correct pin-triple array and check SCK, MOSI and MISO
    // against the RP2040's fixed hardware pin-mux options for the bus
    // (see `firmware/pico/pins.c`). CS is not checked against this table:
    // it's driven as a plain GPIO output, not the SPI peripheral's own
    // CSn line, so any free GPIO is valid for it.
    uint8_t* pin_triples = bus_index == 0 ? SPI_PIN_TRIPLES_BUS_0 : SPI_PIN_TRIPLES_BUS_1;
    if (!spi_pin_check(pin_triples, 0, sclk_pin)) return false;
    if (!spi_pin_check(pin_triples, 1, mosi_pin)) return false;
    if (!spi_pin_check(pin_triples, 2, miso_pin)) return false;

    if (is_pin_taken(sclk_pin) > 0 || is_pin_taken(mosi_pin) > 0 ||
        is_pin_taken(miso_pin) > 0 || is_pin_taken(cs_pin) > 0) {
        return false;
    }

    return true;
}


/**
 * @brief Check that a supplied pin is valid for the board we're using,
 *        for the given SCK/MOSI/MISO role.
 *
 * @param pins:   An array of available pin triples: SCK, MOSI, MISO.
 * @param offset: Which role to check within each triple (0 = SCK,
 *                1 = MOSI, 2 = MISO).
 * @param pin:    The pin to check.
 *
 * @returns Whether the pin is good (`true`) or not (`false`).
 */
// FROM 1.4.0
static bool spi_pin_check(uint8_t* pins, uint8_t offset, uint8_t pin) {

    uint8_t candidate = pins[offset];
    while (candidate != 255) {
        if (candidate == pin) return true;
        pins += 3;
        candidate = pins[offset];
    }

    return false;
}


/**
 * @brief Send SPI bus status information.
 *
 * @param sps: The SPI state record.
 */
void send_spi_status(SPI_State* sps) {

    // Get the RP2040 unique ID
    char pid[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
    pico_get_unique_board_id_string(pid, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
    // e.g., DF6050788B3E1A2E

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
    // Data in the form: "1.1.0.18.19.16.17.1000.1.1.100.110.QTPY-RP2040"
    char status_buffer[SPI_STATUS_BUFFER_SIZE] = {0};
    snprintf(status_buffer, sizeof(status_buffer), "%s.%s.%s.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%i.%s.%s\r\n",
            (sps->is_ready   ? "1" : "0"),                          // 2 chars
            (sps->is_started ? "1" : "0"),                          // 2 chars
            (sps->bus == spi0 ? "0" : "1"),                         // 2 chars
            sps->mosi_pin,                                          // 2-3 chars
            sps->miso_pin,                                          // 2-3 chars
            sps->sclk_pin,                                          // 2-3 chars
            sps->cs_pin,                                            // 2-3 chars
            sps->frequency / 1000,                                  // 2-5 chars (kHz)
            sps->data_size,                                         // 2-3 chars
            sps->cpol,                                              // 2 chars
            sps->cpha,                                              // 2 chars
            major,                                                  // 2-4 chars
            minor,                                                  // 2-4 chars
            patch,                                                  // 2-4 chars
            BUILD_NUM,                                              // 2-4 chars
            pid,                                                    // 17 chars
            model);                                                 // 2-24 chars
                                                                    // Max: 87 chars

    // Send the data
    tx((uint8_t*)status_buffer, strlen(status_buffer));
}


/**
 * @brief Check pin usage.
 *
 * @param sps: The SPI state record.
 * @param pin: An arbitrary GPIO pin that we're checking.
 *
 * @returns `true` if the pin is in use by the bus, or `false`.
 */
bool is_pin_in_use_by_spi(SPI_State* sps, uint8_t pin) {

    return ((pin == sps->sclk_pin || pin == sps->mosi_pin ||
             pin == sps->miso_pin || pin == sps->cs_pin) && sps->is_ready);
}
