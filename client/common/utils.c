/*
 *  macOS/Linux Depot Utility Functions
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "utils.h"


// Declared in each app's main.c
extern SerialDriver board;


/**
 * @brief Issue an error message.
 *
 * @param format_string: Message string with optional formatting.
 * @param ...:           Optional injectable values.
 */
void print_error(char* format_string, ...) {
    va_list args;
    va_start(args, format_string);
    print_output(LOG_TYPE_ERROR, format_string, args);
    va_end(args);
}


/**
 * @brief Issue a warning message.
 *
 * @param format_string: Message string with optional formatting.
 * @param ...:           Optional injectable values.
 */
void print_warning(char* format_string, ...) {
    va_list args;
    va_start(args, format_string);
    print_output(LOG_TYPE_WARNING, format_string, args);
    va_end(args);
}


/**
 * @brief Issue a message.
 *
 * @param format_string: Message string with optional formatting.
 * @param ...:           Optional injectable values.
 */
void print_log(char* format_string, ...) {
    va_list args;
    va_start(args, format_string);
    print_output(LOG_TYPE_MSG, format_string, args);
    va_end(args);
}


/**
 * @brief Issue any message.
 *
 * @param type:          The message type.
 * @param format_string: Message string with optional formatting.
 * @param args:          va_list of args from previous call.
 */
void print_output(uint32_t type, char* format_string, va_list args) {

    // Write the message type to the message
    char buffer[1024] = {0};
    uint32_t delta = 0;
    
    switch(type) {
        case LOG_TYPE_ERROR:
            sprintf(buffer, "[ERROR] ");
            delta = 8;
            break;
        case LOG_TYPE_WARNING:
            sprintf(buffer, "[WARNING] ");
            delta = 10;
            break;
        default:
            break;
    }

    // Write the formatted text to the message
    vsnprintf(&buffer[delta], sizeof(buffer) - delta + 1, format_string, args);

    // Print it all out
    fprintf(stderr, "%s\n", buffer);
}


#ifndef SWIFT_BUILD
/**
 * @brief Callback for Ctrl-C.
 */
void ctrl_c_handler(int dummy) {

    if (board.file_descriptor != -1) serial_flush_and_close_port(&board);
    fprintf(stderr, "\n");
    exit(EXIT_OK);
}
#endif

/**
 * @brief Convert a string to lowercase.
 *
 * @param s: Pointer to the string to convert.
 */
void lower(char* s) {
    size_t l = strlen(s);
    for (int j = 0 ; j < l ; ++j) {
        if (s[j] >= 'A' && s[j] <= 'Z') s[j] += 32;
    }
}
