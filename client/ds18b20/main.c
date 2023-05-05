/*
 * macOS/Linux OneWire CLI utility - DS18B20 version
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Static Prototypes

static inline void  show_help(void);
static inline void  show_version(void);


#pragma mark - Global Vars

// A serial comms structure
SerialDriver board;


#pragma mark - Main Function

/**
 * @brief Main entry point.
 */
int main(int argc, char *argv[]) {

    // Listen for SIGINT
    signal(SIGINT, ctrl_c_handler);

    // Process arguments
    if (argc < 2) {
        // Insufficient arguments -- issue usage info and bail
        fprintf(stderr, "Usage: ds18b20 {DEVICE_PATH} [command]\n");
        return EXIT_OK;
    } else {
        // Check for a help and/or version request
        for (int i = 0 ; i < argc ; ++i) {
            if (strcasecmp(argv[i], "h") == 0 ||
                strcasecmp(argv[i], "--help") == 0 ||
                strcasecmp(argv[i], "-h") == 0) {
                show_help();
                return EXIT_OK;
            }

            if (strcasecmp(argv[i], "v") == 0 ||
                strcasecmp(argv[i], "--version") == 0 ||
                strcasecmp(argv[i], "-v") == 0) {
                show_version();
                return EXIT_OK;
            }
        }

        // Check we have commands to process
        int delta = 2;
        if (argc > delta) {
            // Connect... with the device path
            board.file_descriptor = -1;
            serial_connect(&board, argv[1]);

            if (board.is_connected) {
                // This app requires firmware 1.2 and up
                if (board.fw_version_major == 1 && board.fw_version_minor < 2) {
                    serial_flush_and_close_port(&board);
                    fprintf(stderr, "ds18b20 requires a board with firmware 1.2.0 or above... exiting\n");
                    return EXIT_ERR;
                }

                // Set the mode to 1-Wire
                if (!serial_set_mode(&board, MODE_CODE_ONE_WIRE)) {
                    serial_flush_and_close_port(&board);
                    fprintf(stderr, "Could not set board mode... exiting\n");
                    return EXIT_ERR;
                }

                // Prepare the command byte sequences
                uint8_t cmd_bytes_convert[2] = {0};
                cmd_bytes_convert[0] = 0xCC;
                cmd_bytes_convert[1] = 0x44;

                uint8_t cmd_bytes_read_scratch[2] = {0};
                cmd_bytes_read_scratch[0] = 0xCC;
                cmd_bytes_read_scratch[1] = 0xBE;

                uint8_t result[2] = {0};

                // Start 1-Wire
                if (!one_wire_init(&board)) {
                    serial_flush_and_close_port(&board);
                    fprintf(stderr, "Could not initialise 1-Wire... exiting");
                    return EXIT_ERR;
                }

                fprintf(stderr, "Starting...");

                while (true) {
                    // Send the convert command
                    one_wire_reset(&board);
                    one_wire_write_bytes(&board, cmd_bytes_convert, 2);

                    // Wait 750ms
                    usleep(750 * 1000);

                    // Send the read command
                    one_wire_reset(&board);
                    one_wire_write_bytes(&board, cmd_bytes_read_scratch, 2);

                    // Read back the data
                    one_wire_read_bytes(&board, result, 2);

                    // Calculate the result
                    uint32_t raw_temp = ((uint32_t)(result[1]) * 256) + (uint32_t)(result[0]);
                    double celsius_temp = (double)((raw_temp << 16) >> 16) * 0.0625;

                    // Output the result
                    // NOTE This doesn't use a convenience function as we don't want
                    //      a newline appended, only a carriage return so the text is
                    //      overwritten each time
                    fprintf(stderr, "\rTemperature: %.2f°C   ", celsius_temp);

                    // Wait `READING_INTERVAL_US` between readings
                    usleep(READING_INTERVAL_US);
                }
            }
        }
    }

    if (board.file_descriptor != -1) serial_flush_and_close_port(&board);
    return EXIT_ERR;
}


#pragma mark - User Messaging Functions

/**
 * @brief Show help.
 */
static inline void show_help(void) {

    fprintf(stderr, "ds18b20 {device}\n\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  {device} is a mandatory device path, eg. /dev/cu.usbmodem-101.\n\n");
}


/**
 * @brief Show app version.
 */
static inline void show_version(void) {

    fprintf(stderr, "ds18b20 %s\n", APP_VERSION);
    fprintf(stderr, "Copyright © 2023, Tony Smith.\n");
}
