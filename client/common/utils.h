/*
 * macOS/Linux Depot Utility Functions
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _UTILS_H
#define _UTILS_H


/*
 * INCLUDES
 */
#include <signal.h>
#include "serialdriver.h"


/*
 * CONSTANTS
 */
// FROM 1.1.3
#define LOG_TYPE_MSG                0
#define LOG_TYPE_ERROR              1
#define LOG_TYPE_WARNING            2


/*
 * PROTOTYPES
 */
void    print_warning(char* format_string, ...);
void    print_error(char* format_string, ...);
void    print_log(char* format_string, ...);
void    print_output(uint32_t type, char* format_string, va_list args);
void    ctrl_c_handler(int dummy);
void    lower(char* s);


#endif  // _UTILS_H
