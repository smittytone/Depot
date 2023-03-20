/*
 * Depot RP2040 Bus Host Firmware - Raspberry Pi Pico
 *
 * @version     1.2.1
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include <stdint.h>

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
