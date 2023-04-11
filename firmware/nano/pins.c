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

uint8_t I2C_PIN_PAIRS_BUS_1[] = {   // 6, 7,    // Removed while GPIO 6 is used for mono LED
                                    18, 19,
                                    26, 27,
                                    255, 255};


/*
    Notes on Nano RP2040 Connect Pins
    
    Sensors on i2c0: GPIO 12 (SDA), 13 (SCL),
    but these are also on the pinout.
    
    LSM6DSOXTR Accelerometer INT1 is GPIO 24

    GPIO 22, 23 are the microphone IO

    The Nina WiFi chip uses:
        spi1 
        GPIO 2 - Nina GPIO0
        GPIO 3 - Nina RESET
        GPIO 8 - RX
        GPIO 9 - CS
        GPIO 10 - Nina READY
        GPIO 11 - TX
        GPIO 14 - SCK
*/
