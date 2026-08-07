/*
 * Depot RP2040 Bus Host Firmware - Raspberry Pi Pico
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#include <stdint.h>

// Lists of I2C pin pairs, SDA then SCL

uint8_t I2C_PIN_PAIRS_BUS_0[] = {   0, 1,
                                    4, 5,
                                    8, 9,
                                    12, 13,
                                    16, 17,
                                    20, 21,
                                    255, 255};

uint8_t I2C_PIN_PAIRS_BUS_1[] = {   2, 3,
                                    6, 7,
                                    10, 11,
                                    14, 15,
                                    18, 19,
                                    26, 27,
                                    255, 255};

// FROM 1.4.0
// Lists of SPI pin triples, SCK then MOSI (TX) then MISO (RX).
// CS is not included here: it's driven as a plain GPIO output rather
// than the SPI peripheral's own CSn line, so any free GPIO is valid.
// Derived from the RP2040 GPIO function select table (see the Pico
// SDK's `hardware_gpio` docs) and cross-checked against the official
// Raspberry Pi Pico pinout diagram.

uint8_t SPI_PIN_TRIPLES_BUS_0[] = { 2, 3, 0,
                                    6, 7, 4,
                                    18, 19, 16,
                                    22, 23, 20,
                                    255, 255, 255};

uint8_t SPI_PIN_TRIPLES_BUS_1[] = { 10, 11, 8,
                                    14, 15, 12,
                                    26, 27, 24,
                                    255, 255, 255};
