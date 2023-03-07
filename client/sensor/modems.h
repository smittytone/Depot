/*
 * macOS/Linux 1-Wire DS18B20 readout GUI app - board discovery functions
 *
 * Version 1.2.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#ifndef _MODEM_UTILS_H_
#define _MODEM_UTILS_H_


/*
 * INCLUDES
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>
#include "serialdriver.h"


/*
 * PROTOTYPES
 */
char*   find_boards(unsigned int* device_total);
void    free_string_storage(void);
char*   get_board_info(SerialDriver *sd);


#endif      //  _MODEM_UTILS_H_
