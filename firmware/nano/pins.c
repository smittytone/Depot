/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano 2040 Connect
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include <stdint.h>

uint8_t I2C_PIN_PAIRS_BUS_0[] = {   0, 1,
                                    4, 5,
                                    12, 13,     // Connected to board's I2C sensors
                                    16, 17,
                                    20, 21,
                                    255, 255};

uint8_t I2C_PIN_PAIRS_BUS_1[] = {   6, 7,
                                    18, 19,
                                    26, 27,
                                    255, 255};


/*
Accelerometer INT is GPIO 24

GPIO 22, 23 are the microphone IO

SPI1 8,9,10,11,14 is the Nina link
GPIO 3 is Nina RESET
GPIO 2 is Nina GPIO0
*/
