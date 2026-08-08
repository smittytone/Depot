/*
 * Depot RP2040 Bus Host Firmware - Errors list
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#ifndef _ERRORS_HEADER_
#define _ERRORS_HEADER_

/*
    Error format:

    Bits 8-6: Mode
    Bits 5-1: Code

    General: 000-xxxxx      0
    I2C:     001-xxxxx      1
    SPI:     010-xxxxx      2
    UART:    011-xxxxx      3
    1-Wire:  100-xxxxx      4
    GPIO:    101-xxxxx      5

    By type:
    WRITE           2
    READ            3
    CONFIG          4
    PINS IN USE     5
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
    I2C_COULD_NOT_CONFIGURE     = 0x24,
    I2C_PINS_ALREADY_IN_USE     = 0x25,

    // SPI
    SPI_NOT_READY               = 0x40,
    SPI_NOT_STARTED             = 0x41,
    SPI_COULD_NOT_WRITE         = 0x42,
    SPI_COULD_NOT_READ          = 0x43,
    SPI_COULD_NOT_CONFIGURE     = 0x44,
    SPI_PINS_ALREADY_IN_USE     = 0x45,
    SPI_UNAVAILABLE_ON_BOARD    = 0x49,

    // UART
    UART_NOT_READY              = 0x60,

    // ONE-WIRE
    OW_NOT_READY                = 0x80,
    OW_NO_DEVICES_FOUND         = 0x81,
    OW_COULD_NOT_WRITE          = 0x82,
    OW_COULD_NOT_READ           = 0x83,
    OW_COULD_NOT_CONFIGURE      = 0x84,
    OW_PINS_ALREADY_IN_USE      = 0x85,

    GPIO_NOT_READY              = 0xA0,
    // = 0xA1
    // = 0xA2
    // = 0xA3
    GPIO_CANT_SET_PIN           = 0xA4,
    GPIO_PIN_ALREADY_IN_USE     = 0xA5,
    GPIO_PIN_ILLEGAL_VALUE      = 0xA9,

    // DO NOT USE VALUE 0xF0
    GEN_DO_NOT_USE_ERR          = 0xF0,
};


#endif  // _ERRORS_HEADER_
