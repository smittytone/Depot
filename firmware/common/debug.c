/*
 * Depot RP2040 Bus Host Firmware - Debug functions
 *
 * @version     1.2.4
 * @author      Tony Smith (@smittytone)
 * @copyright   2026
 * @licence     MIT
 *
 */
// C
#include <stdio.h>
// Pico
#include "hardware/uart.h"
// Depot
#include "debug.h"


/**
 * @brief Initialise UART and pins for debugging output.
 */
void debug_init(void) {
    uart_init(DEBUG_UART, 115200);
    gpio_set_function(DEBUG_UART_RX_GPIO, GPIO_FUNC_UART);
    gpio_set_function(DEBUG_UART_TX_GPIO, GPIO_FUNC_UART);
    uart_puts(DEBUG_UART, "Logging...\r\n");
}


/**
 * @brief Post a debug log message to UART.
 *
 * @param format_string: Message string with optional formatting
 * @param ...:           Optional injectable values
 */
void debug_log(char* format_string, ...) {

    va_list args;
    char buffer[DEBUG_MESSAGE_MAX_B] = {0};
    size_t buffer_size = sizeof(buffer) - 1;

    uint32_t ts = to_ms_since_boot(get_absolute_time());
    snprintf(buffer, buffer_size, "%i ", ts);
    size_t len = strlen(buffer);

    // Compile the string
    va_start(args, format_string);
    vsnprintf(buffer + len, buffer_size - 2 - len, format_string, args);
    va_end(args);

    // Issue the compiled string and EOL markers to UART
    uart_puts(DEBUG_UART, buffer);
    uart_puts(DEBUG_UART, "\r\n");
}


void debug_log_bytes(uint8_t* data, size_t count) {

    char buffer[DEBUG_MESSAGE_MAX_B] = {0};
    size_t buffer_size = sizeof(buffer) - 1;

    uint32_t ts = to_ms_since_boot(get_absolute_time());
    snprintf(buffer, buffer_size, "%i ", ts);
    size_t len = strlen(buffer);

    for (size_t i = 0 ; i < count ; ++i) {
        size_t offset = i * 2 + len;
        snprintf(buffer + offset, buffer_size - offset, "%02X", data[i]);
    }

    // Issue the compiled string and EOL markers to UART
    uart_puts(DEBUG_UART, buffer);
    uart_puts(DEBUG_UART, "\r\n");
}
