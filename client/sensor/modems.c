/*
 * macOS/Linux 1-Wire DS18B20 readout GUI app - board discovery functions
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "modems.h"


/*
 * GLOBALS
 */
char* string_store = NULL;


/**
 * @brief Get a list of serial devices connected to the host.
 *
 * @param board_total: Pointer to a UInt into which the number of
 *                     boards detected will be written.
 *
 * @returns A pointer to the data string, which must be
 *         released once processed by calling `free_string_storage()`.
 */
char* find_boards(unsigned int* board_total) {

    io_iterator_t   serial_port_iterator;
    io_object_t     modem_service;
    uint32_t        board_count = 0;
    char            bsd_path[MAXPATHLEN] = {0};
    char            boards[MAXPATHLEN * 10] = {0};
    char*           store_ptr = boards;

    CFMutableDictionaryRef classes_to_match = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classes_to_match != NULL) {
        // Look for boards that claim to be modems.
        CFDictionarySetValue(classes_to_match,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDModemType));
    }

    // Get an iterator across all matching boards.
    kern_return_t kern_result = IOServiceGetMatchingServices(kIOMasterPortDefault, classes_to_match, &serial_port_iterator);
    if (kern_result != KERN_SUCCESS) return 0;

    // Iterate across all modems found
    while ((modem_service = IOIteratorNext(serial_port_iterator))) {

        // Get the callout device's path (/dev/cu.xxxxx)
        CFTypeRef bsd_path_as_CFString = IORegistryEntryCreateCFProperty(modem_service,
                                                                         CFSTR(kIOCalloutDeviceKey),
                                                                         kCFAllocatorDefault,
                                                                         0);
        if (bsd_path_as_CFString) {
            // Convert the path from a CFString to a C (NUL-terminated) string
            Boolean result = CFStringGetCString(bsd_path_as_CFString,
                                                bsd_path,
                                                MAXPATHLEN,
                                                kCFStringEncodingUTF8);
            CFRelease(bsd_path_as_CFString);

            // If we have a valid path, and the path is not for Mac BLE devices,
            // or AirPods etc., add it to the data string.
            if (result && strstr(bsd_path, "usbmodem") != NULL) {
                sprintf(store_ptr, "%s|", bsd_path);
                store_ptr += (strlen(bsd_path) + 1);
                board_count++;
                if (board_count > 9) break;
            }
        }
    }

    // Release objects we're done with
    (void)IOObjectRelease(modem_service);
    IOObjectRelease(serial_port_iterator);

    // Pass back the device count
    *board_total = board_count;

    // Write the boards string to the heap and
    // return a pointer to it. Swift will decode it
    //string_store = (char*)malloc(strlen(boards) + 2);
    //strcpy(string_store, boards);
    string_store = strdup(boards);
    return string_store;
}


/**
 * @brief Free string storage once it has been read.
 */
void free_string_storage(void) {

    if (string_store != NULL) free(string_store);
}


/**
 * @brief Get info from the board.
 *        Note that this bypasses the driver function.
 *
 * @param sd:       Pointer to a SerialDriver structure.
 *
 * @returns A pointer to the data string, which must be
 *         released once processed by calling `free_string_storage()`.
 */
char* get_board_info(SerialDriver *sd) {

    uint8_t read_buffer[129] = {0};
    serial_send_command(sd, '?');
    size_t result = serial_read_from_port(sd->file_descriptor, read_buffer, 0);
    if (result == -1) return NULL;
    read_buffer[result] = 0;

    string_store = strdup((char*)read_buffer);
    return string_store;
}
