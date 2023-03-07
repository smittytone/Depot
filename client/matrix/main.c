/*
 * I2C driver for an HT16K33 8x8 Matrix Display
 *
 * Version 1.1.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Static Prototypes

static int          process_commands(SerialDriver* sd, int argc, char* argv[], int delta);
static void         show_help(void);
static inline void  show_version(void);


#pragma mark - Global Vars

// A serial comms structure
SerialDriver board;
I2CData i2c_data;


#pragma mark - Main Function

/**
 * @brief Main entry point.
 */
int main(int argc, char* argv[]) {

    // Listen for SIGINT
    signal(SIGINT, ctrl_c_handler);

    // Process arguments
    if (argc < 2) {
        // Insufficient arguments -- issue usage info and bail
        fprintf(stderr, "Usage: matrix {DEVICE_PATH} [I2C Address] [command] ... [command]\n");
        return EXIT_OK;
    } else {
        // Check for a help request
        for (int i = 0 ; i < argc ; ++i) {
            if (strcasecmp(argv[i], "h") == 0 ||
                strcasecmp(argv[i], "-h") == 0 ||
                strcasecmp(argv[i], "--help") == 0) {
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

        // Connect... with the device path
        i2c_data.address = HT16K33_I2C_ADDR;
        board.file_descriptor = -1;
        serial_connect(&board, argv[1]);
        if (board.is_connected) {
            // Set the mode to I2C -- requires firmware 1.2 and up
            if (board.fw_version_minor > 1 && !serial_set_mode(&board, MODE_CODE_I2C)) {
                serial_flush_and_close_port(&board);
                fprintf(stderr, "Could not set board mode... exiting\n");
                return EXIT_ERR;
            }

            // Initialize the I2C host's I2C bus
            if (!(i2c_init(&board))) {
                print_error("%s could not initialise I2C", argv[1]);
                serial_flush_and_close_port(&board);
                return EXIT_ERR;
            }

            // Process the remaining commands in sequence
            int delta = 2;
            if (argc > delta) {
                char* token = argv[2];
                if (token[0] >= '0' && token[0] <= '9') {
                    // Not a command, so an address?
                    i2c_data.address = (int)strtol(token, NULL, 0);

                    // Only allow legal I2C address range
                    if (i2c_data.address < 0x08 || i2c_data.address > 0x77) {
                        print_error("I2C address out of range");
                        serial_flush_and_close_port(&board);
                        return EXIT_ERR;
                    }

                    // Step past handled args
                    delta = 3;
                }

                // Set up the display driver
                HT16K33_init(&board, &i2c_data, HT16K33_0_DEG);

                // Process the commands one by one
                int result = process_commands(&board, argc, argv, delta);
                serial_flush_and_close_port(&board);
                return result;
            } else {
                fprintf(stderr, "No commands supplied... exiting\n");
                serial_flush_and_close_port(&board);
                return EXIT_OK;
            }
        }
    }

    if (board.file_descriptor != -1) serial_flush_and_close_port(&board);
    return EXIT_ERR;
}


#pragma mark - Command Parsing and Processing

/**
 * @brief Parse and process commands for the HT16K33-based matrix.
 *
 * @param sd:    A SerialDriver data structure.
 * @param argc:  The argument count.
 * @param argv:  The arguments.
 * @param delta: The argument list offset to locate HT16K33 commands from.
 */
static int process_commands(SerialDriver* sd, int argc, char* argv[], int delta) {

    bool do_draw = false;

    for (int i = delta ; i < argc ; ++i) {
        // Get the next command
        char* command = argv[i];
        char cmd = command[0];

        // Check for a switch marker
        if (cmd == '-' && strlen(command) > 1) cmd = command[1];

        // Look up the command
        switch (cmd) {
            case 'A':
            case 'a':   // ACTIVATE (DEFAULT) OR DEACTIVATE DISPLAY
                        // 1 parameter: on|off
                {
                    // Check for and get the optional argument
                    bool is_on = true;
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (strcmp(command, "off") == 0) {
                            is_on = false;
                        } else if (strcmp(command, "on") != 0) {
                            i -= 1;
                        }
                    }

                    // Apply the command
                    HT16K33_power(is_on);
                }
                break;

            case 'B':
            case 'b':   // SET BRIGHTNESS
                        // 1 parameter: 0-15
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            long brightness = strtol(command, NULL, 0);

                            if (brightness < 0 || brightness > 15) {
                                print_error("Brightness value out of range (0-15)");
                                return EXIT_ERR;
                            }

                            // Apply the command
                            HT16K33_set_brightness(brightness);
                            break;
                        }
                    }

                    print_error("No brightness value supplied");
                    return EXIT_ERR;
                }

            case 'C':
            case 'c':   // DISPLAY A CHARACTER
                        // 2 parameters: 0-255, true|false
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            uint8_t achar = (uint8_t)strtol(command, NULL, 0);

                            if (achar < 32 || achar > 127) {
                                print_error("Character out of range (Ascii 32-127)");
                                return EXIT_ERR;
                            }

                            // Get an optional argument
                            bool do_centre = false;
                            if (i < argc - 1) {
                                command = argv[++i];
                                if (strcmp(command, "true") == 0) {
                                    do_centre = true;
                                } else if (strcmp(command, "false") != 0) {
                                    i -= 1;
                                }
                            }

                            // Perform the action
                            HT16K33_set_char(achar, do_centre);
                            do_draw = true;
                            break;
                        }
                    }

                    print_error("No Ascii value supplied");
                    return EXIT_ERR;
                }

            case 'G':
            case 'g':   // DISPLAY A GLYPH
                        // 1 parameter: string hex sequence
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] == '0' && command[1] == 'x') {
                            uint8_t bytes[8] = {0};
                            char *endptr = command;
                            size_t length = 0;

                            while (length < sizeof(bytes)) {
                                bytes[length++] = (uint8_t)strtol(endptr, &endptr, 0);
                                if (*endptr == '\0') break;
                                if (*endptr != ',') {
                                    print_error("Invalid bytes");
                                    return EXIT_ERR;
                                }

                                endptr++;
                            }

                            // Perform the action
                            HT16K33_set_glyph(bytes);
                            do_draw = true;
                            break;
                        }
                    }

                    print_error("No glyph value supplied");
                    return EXIT_ERR;
                }

            case 'P':
            case 'p':   // PLOT A POINT
                        // 3 parameters: 0-7, 0-7, 1|0
                {
                    // Get two required arguments
                    long x = -1, y = -1;
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            x = strtol(command, NULL, 0);
                            if (i < argc - 1) {
                                command = argv[++i];
                                if (command[0] >= '0' && command[0] <= '9') {
                                    y = strtol(command, NULL, 0);

                                    if (x < 0 || x > 7 || y < 0 || y > 7) {
                                        print_error("Co-ordinate out of range (0-7)");
                                        return EXIT_ERR;
                                    }

                                    // Get an optional argument
                                    long ink = 1;
                                    if (i < argc - 1) {
                                        command = argv[++i];
                                        if (command[0] == '0' || command[0] == '1') {
                                            ink = strtol(command, NULL, 0);
                                            if (ink != 1 && ink != 0) ink = 1;
                                        } else {
                                            i -= 1;
                                        }
                                    }

                                    // Perform the action
                                    HT16K33_plot((uint8_t)x, (uint8_t)y, ink == 1);
                                    do_draw = true;
                                    break;
                                }
                            }
                        }
                    }

                    print_error("No co-ordinate value(s) supplied");
                    return EXIT_ERR;
                }
                break;

            case 'R':
            case 'r':   // ROTATE DISPLAY
                        // 1 parameter: 0-3 (x 90 degrees)
                {
                    // Get an optional argument
                    long angle = 0;
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            angle = strtol(command, NULL, 0);
                        } else {
                            i -= 1;
                        }
                    }

                    // Perform the action
                    HT16K33_set_angle((uint8_t)angle);
                    HT16K33_rotate((uint8_t)angle);
                }
                break;

            case 'T':
            case 't':   // SCROLL A TEXT STRING
                        // 2 parameters: string to scroll, delay
                {
                    // Get one required argument
                    if (i < argc - 1) {
                        // Assume the whole next arg is the string
                        char *scroll_string = argv[++i];

                        // Get an optional argument
                        uint32_t scroll_delay_ms = 100;
                        if (i < argc - 1) {
                            command = argv[++i];
                            if (command[0] >= '0' && command[0] <= '9') {
                                scroll_delay_ms = (uint32_t)strtol(command, NULL, 0);
                            } else {
                                i -= 1;
                            }
                        }

                        // Perform the action
                        HT16K33_print(scroll_string, scroll_delay_ms);
                        break;
                    }

                    print_error("No string supplied");
                    return EXIT_ERR;
                }

            case 'W':
            case 'w':   // WIPE (CLEAR) THE DISPLAY
                        // No parameters
                HT16K33_clear_buffer();
                do_draw = true;
                break;

            case 'Z':
            case 'z':   // DRAW THE DISPLAY IMMEDIATELY
                        // No parameters
                HT16K33_draw(false);
                do_draw = false;
                break;

            default:
                // ERROR
                print_error("Unknown command");
                return EXIT_ERR;
        }
    }

    if (do_draw) HT16K33_draw(false);
    return EXIT_OK;
}


#pragma mark - User Messaging Functions

/**
 * @brief Show help.
 */
static void show_help(void) {

    fprintf(stderr, "matrix {device} [address] [commands]\n\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  {device} is a mandatory device path, eg. /dev/cu.usbmodem-010101.\n");
    fprintf(stderr, "  [address] is an optional display I2C address. Default: 0x70.\n");
    fprintf(stderr, "  [commands] are optional HT16K33 matrix commands:\n\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  a [on|off]             Activate/deactivate the display. Default: on.\n");
    fprintf(stderr, "  b {0-15}               Set the display brightness from low (0) to high (15).\n");
    fprintf(stderr, "  r {0-3}                Rotate the display. Angle supplied as a multiple of 90 degrees.\n");
    fprintf(stderr, "  c {ascii} [true|false] Draw the Ascii character on the screen, and optionally\n");
    fprintf(stderr, "                         set it to be centred (true).\n");
    fprintf(stderr, "  g {glyph}              Draw the user-defined character on the screen. The definition\n");
    fprintf(stderr, "                         is a string of eight comma-separated 8-bit hex values, eg.\n");
    fprintf(stderr, "                         '0x3C,0x42,0xA9,0x85,0x85,0xA9,0x42,0x3C'.\n");
    fprintf(stderr, "  p {x} {y} [1|0]        Set or clear the specified pixel. X and Y coordinates are in\n");
    fprintf(stderr, "                         the range 0-7.\n");
    fprintf(stderr, "  t {string} [delay]     Scroll the specified string. The second argument is an optional\n");
    fprintf(stderr, "                         delay be between column shifts in milliseconds. Default: 250ms.\n");
    fprintf(stderr, "  w                      Wipe (clear) the display.\n");
    fprintf(stderr, "  h                      Help information.\n\n");
}


/**
 * @brief Show app version.
 */
static inline void show_version(void) {

    fprintf(stderr, "matrix %s\n", APP_VERSION);
    fprintf(stderr, "Copyright © 2023, Tony Smith.\n");
}
