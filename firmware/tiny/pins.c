/*
 * Depot RP2040 Bus Host Firmware - Pimoroni Tiny 2040
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include <stdint.h>

uint8_t I2C_PIN_PAIRS_BUS_0[] = {   0, 1,
                                    4, 5,
                                    255, 255};

uint8_t I2C_PIN_PAIRS_BUS_1[] = {   2, 3,
                                    6, 7,
                                    26, 27,
                                    255, 255};
