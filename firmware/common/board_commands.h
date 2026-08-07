/*
 * Depot RP2040 Bus Host Firmware - Primary command list
 *
 * @version     1.4.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
#ifndef _HEADER_COMMANDS_
#define _HEADER_COMMANDS_


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
#define CMD_GPIO_SET_READ_WRITE                 'g'
// FROM 1.4.0 -- these are now also used to select/deselect an SPI
// peripheral by asserting/de-asserting its CS line
#define CMD_MULTIBUS_START                      's'
#define CMD_MULTIBUS_STOP                       'p'


#endif // _HEADER_COMMANDS_
