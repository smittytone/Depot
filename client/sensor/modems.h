/*
 * macOS/Linux 1-Wire DS18B20 readout GUI app - board discovery functions
 *
 * Version 1.2.3
 * Copyright © 2025, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _MODEM_UTILS_H_
#define _MODEM_UTILS_H_


/*
 * INCLUDES
 */
#include "serialdriver.h"


/*
 * PROTOTYPES
 */
char*   find_boards(unsigned int* device_total);
void    free_string_storage(void);
char*   get_board_info(SerialDriver *sd);


#endif      //  _MODEM_UTILS_H_
