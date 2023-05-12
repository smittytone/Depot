/*
 * Depot RP2040 Bus Host Firmware - Errors list
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _ERRORS_HEADER_
#define _ERRORS_HEADER_

/*
    Error format:

    Bits 8-6: Mode
    Bits 5-1: Code

    General: 000-xxxxx
    I2C:     001-xxxxx
    SPI:     010-xxxxx
    UART:    011-xxxxx
    1-Wire:  100-xxxxx
    GPIO:    101-xxxxx
*/

enum HOST_ERRORS {
    GEN_NO_ERROR                = 0x00,
    GEN_UNKNOWN_MODE            = 0x01,
    GEN_UNKNOWN_COMMAND         = 0x02,
    GEN_LED_NOT_ENABLED         = 0x03,
    GEN_CANT_CONFIG_BUS         = 0x04,
    GEN_CANT_GET_BUS_INFO       = 0x05,

    // DO NOT USE VALUE 0x0F
    GEN_DO_NOT_USE_ACK          = 0x0F,

    // I2C
    I2C_NOT_READY               = 0x20,
    I2C_NOT_STARTED             = 0x21,
    I2C_COULD_NOT_WRITE         = 0x22,
    I2C_COULD_NOT_READ          = 0x23,
    I2C_ALREADY_STOPPED         = 0x24,
    I2C_COULD_NOT_CONFIGURE     = 0x25,
    I2C_PINS_ALREADY_IN_USE     = 0x26,

    // SPI
    SPI_NOT_STARTED             = 0x40,
    SPI_COULD_NOT_WRITE         = 0x41,
    SPI_COULD_NOT_READ          = 0x42,
    SPI_UNAVAILABLE_ON_BOARD    = 0x43,

    // ONE-WIRE
    OW_NOT_READY                = 0x80,
    OW_NO_DEVICES_FOUND         = 0x81,
    OW_COULD_NOT_READ           = 0x82,
    // = 0x83
    // = 0x84
    OW_COULD_NOT_CONFIGURE      = 0x85,
    OW_PIN_ALREADY_IN_USE       = 0x86,

    GPIO_ILLEGAL_PIN            = 0xA0,
    // = 0xA1
    // = 0xA2
    // = 0xA3
    // = 0xA4
    GPIO_CANT_SET_PIN           = 0xA5,
    GPIO_PIN_ALREADY_IN_USE     = 0xA6,
    GPIO_CANT_SET_BUTTON        = 0xA7,


    // DO NOT USE VALUE 0xF0
    GEN_DO_NOT_USE_ERR          = 0xF0,
};


#endif  // _ERRORS_HEADER_
