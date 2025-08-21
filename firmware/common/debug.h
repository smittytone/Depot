/*
 * Depot RP2040 Bus Host Firmware - Debug functions
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2025
 * @licence     MIT
 *
 */
#ifndef _DEBUG_HEADER_
#define _DEBUG_HEADER_




/*
 * CONSTANTS
 */
#define DEBUG_UART_RX_GPIO                      17
#define DEBUG_UART_TX_GPIO                      16
#define DEBUG_UART                              uart0
#define DEBUG_MESSAGE_MAX_B                     512

/*
 * PROTOTYPES
 */
void    debug_init(void);
void    debug_log(char* format_string, ...);
void    debug_log_bytes(uint8_t* bytes, size_t count);


#endif  // _DEBUG_HEADER_
