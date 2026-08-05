/*
 * Depot RP2040 Bus Host Firmware - Primary serial and command functions
 *
 * @version     1.3.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#ifndef _HEADER_SERIAL_
#define _HEADER_SERIAL_


/*
 * INCLUDES
 */
#include <stdint.h>

#ifdef DO_UART_DEBUG
#include "debug.h"
#endif


/*
 * CONSTANTS
 */
#define SERIAL_READ_TIMEOUT_US                  10
#define RX_LOOP_DELAY_MS                        5
#define HEARTBEAT_PERIOD_US                     2000000
#define HEARTBEAT_FLASH_US                      50000

#define WRITE_LENGTH_BASE                       0xC0
#define READ_LENGTH_BASE                        0x80

#define HW_MODEL_NAME_SIZE_MAX                  24

#ifndef DEBUG_SEG_ADDR
// Just in case the user comments out the CMakeLists.txt define
#define DEBUG_SEG_ADDR                          0x70
#endif

#define ACK                                     0x0F
#define ERR                                     0xF0

// FROM 1.1.2
#define UART_LOOP_DELAY_MS                      1
#define RX_BUFFER_LENGTH_B                      128

// FROM 1.1.3
#define MODE_CODE_NONE                          '0'
#define MODE_CODE_I2C                           'i'
#define MODE_CODE_SPI                           's'
#define MODE_CODE_UART                          'u'
#define MODE_CODE_ONE_WIRE                      'o'
#define MODE_CODE_ONE_WIRE_ALT                  '1'

#define MAX_NUMBER_OF_MODES                     4

#define COLOUR_MODE_I2C                         0x002010 // Cyan
#define COLOUR_MODE_SPI                         0x010000 //0x100010
#define COLOUR_MODE_UART                        0x010000 //0x001000
#define COLOUR_MODE_ONE_WIRE                    0x101000 // Yellow
#define COLOUR_MODE_ONE_NONE                    0x100000 // Red

#define ERROR_BUFFER_LENGTH_B                   129
#define BUS_RX_BUFFER_LENGTH_B                  65

#define PIN_USAGE_FIELD_GPIO                    0x01
#define PIN_USAGE_FIELD_I2C                     0x02
#define PIN_USAGE_FIELD_ONEWIRE                 0x10

#define ERR_MSG_SIZE_BYTES                      3

#define CMD_REQUEST_CONN                        '!'
#define CMD_SET_LED_STATE                       '*'
#define CMD_GET_STATUS                          '?'
#define CMD_GET_LAST_ERROR                      '$'
#define CMD_SET_MODE                            '#'
#define CMD_MULTIBUS_CONFIGURE_BUS              'c'
#define CMD_MULTIBUS_DEVICE_SCAN                'd'
#define CMD_MULTIBUS_INIT_BUS                   'i'
#define CMD_MULTIBUS_RESET_BUS                  'x'
#define CMD_MULTIBUS_DEINIT_BUS                 'k'
#define CMD_I2C_SET_100KHZ                      '1'
#define CMD_I2C_SET_400KHZ                      '4'
#define CMD_I2C_START                           's'
#define CMD_I2C_STOP                            'p'
#define CMD_GPIO_SET_READ_WRITE                 'g'


/*
 * PROTOTYPES
 */
void        rx_loop(void);
void        tx(uint8_t* buffer, uint32_t byte_count);
uint8_t     is_pin_taken(uint32_t pin);


#endif  // _HEADER_SERIAL_
