/*
 * Depot RP2040 Bus Host Firmware - SPI functions
 *
 * @version     1.4.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#ifndef _HEADER_SPI_
#define _HEADER_SPI_


/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdint.h>


/*
 * CONSTANTS
 */
#ifndef     DEFAULT_SPI_BUS
#define     DEFAULT_SPI_BUS                         0
#endif
#define     SPI_STATUS_BUFFER_SIZE                  129

// FROM 1.4.0
// Byte offsets of each field within the payload sent with the
// `CMD_MULTIBUS_CONFIGURE_BUS` ('c') command, when the current
// mode is `MODE_CODE_SPI`. Byte 0 is the bus ID (bit 0 only).
#define     SPI_CONFIG_BYTE_MOSI_PIN                 1
#define     SPI_CONFIG_BYTE_MISO_PIN                 2
#define     SPI_CONFIG_BYTE_SCLK_PIN                 3
#define     SPI_CONFIG_BYTE_CS_PIN                   4
#define     SPI_CONFIG_BYTE_CPOL                     5
#define     SPI_CONFIG_BYTE_CPHA                     6
#define     SPI_CONFIG_BYTE_DATA_SIZE                7

/*
 * STRUCTURES
 */
typedef struct {
    bool        is_ready;
    bool        is_started;
    uint8_t     sclk_pin;
    uint8_t     mosi_pin;
    uint8_t     miso_pin;
    uint8_t     cs_pin;
    uint8_t     cpol;
    uint8_t     cpha;
    uint8_t     data_size;
    uint32_t    frequency;              // Baud rate, in Hz
    uint32_t    read_byte_count;
    uint32_t    write_byte_count;
    spi_inst_t* bus;
} SPI_State;


/*
 * PROTOTYPES
 */
void    init_spi(SPI_State* sps);
void    deinit_spi(SPI_State* sps);
void    reset_spi(SPI_State* sps);
void    set_spi_frequency(SPI_State* sps, uint32_t frequency_khz);
bool    configure_spi(SPI_State* sps, uint8_t* data);
void    send_spi_status(SPI_State* sps);
bool    is_pin_in_use_by_spi(SPI_State* sps, uint8_t pin);


#endif  // _HEADER_SPI_
