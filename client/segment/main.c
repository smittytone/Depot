/*
 * I2C driver for an HT16K33 4-digit, 7-segment display
 *
 * Version 1.2.3
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
        fprintf(stderr, "Usage: segment {DEVICE_PATH} [I2C Address] [command] ... [command]\n");
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
                HT16K33_init(&board, &i2c_data);

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
                        // 0|1 parameter: on|off
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
            case 'c':   // DISPLAY A CHARACTER ON A DIGIT
                        // 3 parameters: 0-255, 0-3, true|false
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (strlen(command) == 1) {
                            char achar = command[0];

                            // Get a required argument
                            if (i < argc - 1) {
                                command = argv[++i];
                                if (command[0] >= '0' && command[0] <= '9') {
                                    uint8_t digit = (uint8_t)strtol(command, NULL, 0);

                                    if (digit > 3) {
                                        print_error("Digit value out of range (0-3)");
                                        return EXIT_ERR;
                                    }

                                    // Get an optional argument
                                    bool show_point = false;
                                    if (i < argc - 1) {
                                        command = argv[++i];
                                        if (strcmp(command, "true") == 0) {
                                            show_point = true;
                                        } else if (strcmp(command, "false") != 0) {
                                            i -= 1;
                                        }
                                    }

                                    // Perform the action
                                    HT16K33_set_char(achar, digit, show_point);
                                    do_draw = true;
                                    break;
                                }
                            }

                            print_error("No digit value supplied");
                            return EXIT_ERR;
                        }
                    }

                    print_error("No character supplied");
                    return EXIT_ERR;
                }

            case 'D':
            case 'd':   // SET DECIMAL POINT ON A DIGIT (0-3)
                        // 1 parameter: 0-3
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            uint8_t digit = (uint8_t)strtol(command, NULL, 0);

                            if (digit > 3) {
                                print_error("Digit value out of range (0-3)");
                                return EXIT_ERR;
                            }

                            // Apply the command
                            HT16K33_set_point(digit);
                            do_draw = true;
                            break;
                        }
                    }

                    print_error("No digit value supplied");
                    return EXIT_ERR;
                }

            case 'F':
            case 'f':   // FLIP DISPLAY
                        // No parameters
                HT16K33_flip();
                break;

            case 'G':
            case 'g':   // DISPLAY A GLYPH ON A DIGIT
                        // 3 parameters: 0-255, 0-3, true|false
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if (command[0] >= '0' && command[0] <= '9') {
                            uint8_t glyph = (uint8_t)strtol(command, NULL, 0);

                            if (glyph > 0xFF) {
                                print_error("Glyph value out of range (0-255)");
                                return EXIT_ERR;
                            }

                            // Get a required argument
                            if (i < argc - 1) {
                                command = argv[++i];
                                if (command[0] >= '0' && command[0] <= '9') {
                                    uint8_t digit = (uint8_t)strtol(command, NULL, 0);

                                    if (digit > 3) {
                                        print_error("Digit value out of range (0-3)");
                                        return EXIT_ERR;
                                    }

                                    // Get an optional argument
                                    bool show_point = false;
                                    if (i < argc - 1) {
                                        command = argv[++i];
                                        if (strcmp(command, "true") == 0) {
                                            show_point = true;
                                        } else if (strcmp(command, "false") != 0) {
                                            i -= 1;
                                        }
                                    }

                                    // Perform the action
                                    HT16K33_set_glyph(glyph, digit, show_point);
                                    do_draw = true;
                                    break;
                                }
                            }

                            print_error("No digit value supplied");
                            return EXIT_ERR;
                        }
                    }

                    print_error("No glyph value supplied");
                    return EXIT_ERR;
                }

            case 'K':
            case 'k':   // SET OR UNSET THE COLON
                        // No parameters
                HT16K33_set_colon();
                do_draw = true;
                break;

            case 'N':
            case 'n':   // DISPLAY A NUMBER ACROSS THE DISPLAY
                        // 1 parameter: -999 to 9999
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if ((command[0] >= '0' && command[0] <= '9') ||
                            (command[0] == '-' && command[1] >= '0' && command[1] <= '9')) {
                            int number = (int)strtol(command, NULL, 0);

                            if (number < -999 || number > 9999) {
                                print_error("Decimal value out of range (-999 to 9999)");
                                return EXIT_ERR;
                            }

                            // Perform the action
                            HT16K33_show_value(number, false);
                            do_draw = true;
                            break;
                        }
                    }

                    print_error("No number supplied");
                    return EXIT_ERR;
                }

            case 'V':
            case 'v':   // DISPLAY A VALUE ON A DIGIT
                        // 3 parameters: 0-15, 0-3, true|false
                {
                    // Get the required argument
                    if (i < argc - 1) {
                        command = argv[++i];
                        if ((command[0] >= '0' && command[0] <= '9') ||
                            (command[0] >= 'a' && command[0] <= 'f')) {
                            uint8_t number = (uint8_t)strtol(command, NULL, 0);

                            if (number > 0x0F) {
                                print_error("Value out of range (00-0F)");
                                return EXIT_ERR;
                            }

                            // Get a required argument
                            if (i < argc - 1) {
                                command = argv[++i];
                                if (command[0] >= '0' && command[0] <= '9') {
                                    uint8_t digit = (uint8_t)strtol(command, NULL, 0);

                                    if (digit > 3) {
                                        print_error("Digit value out of range (0-3)");
                                        return EXIT_ERR;
                                    }

                                    // Get an optional argument
                                    bool show_point = false;
                                    if (i < argc - 1) {
                                        command = argv[++i];
                                        if (strcmp(command, "true") == 0) {
                                            show_point = true;
                                        } else if (strcmp(command, "false") != 0) {
                                            i -= 1;
                                        }
                                    }

                                    // Perform the action
                                    HT16K33_set_number(number, digit, show_point);
                                    do_draw = true;
                                    break;
                                }
                            }

                            print_error("No digit value supplied");
                            return EXIT_ERR;
                        }
                    }

                    print_error("No numeric value supplied");
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

    if (do_draw) HT16K33_draw(true);
    return EXIT_OK;
}


#pragma mark - User Messaging Functions

/**
 * @brief Show help.
 */
static void show_help(void) {

    fprintf(stderr, "segment {device} [address] [commands]\n\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  {device} is a mandatory device path, eg. /dev/cu.usbmodem-010101.\n");
    fprintf(stderr, "  [address] is an optional display I2C address. Default: 0x70.\n");
    fprintf(stderr, "  [commands] are optional HT16K33 segment commands.\n\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  a [on|off]                      Activate/deactivate the display. Default: on.\n");
    fprintf(stderr, "  b {0-15}                        Set the display brightness from low (0) to high (15).\n");
    fprintf(stderr, "  f                               Flip the display vertically.\n");
    fprintf(stderr, "  n {number}                      Draw the decimal number on the screen.\n");
    fprintf(stderr, "                                  Range -999 to 9999.\n");
    fprintf(stderr, "  v {value} {digit} [true|false]  Draw the value on the screen at the specified digit\n");
    fprintf(stderr, "                                  (0-15/0x00-0x0F) and optionally set its decimal point.\n");
    fprintf(stderr, "  g {glyph} {digit} [true|false]  Draw the user-defined character on the screen at the\n");
    fprintf(stderr, "                                  specified digit. The glyph definition is a byte with bits\n");
    fprintf(stderr, "                                  set for each of the digit’s segments.\n");
    fprintf(stderr, "  w                               Wipe (clear) the display.\n");
    fprintf(stderr, "  h                               Help information.\n\n");
}


/**
 * @brief Show app version.
 */
static inline void show_version(void) {

    fprintf(stderr, "segment %s\n", APP_VERSION);
    fprintf(stderr, "Copyright © 2023, Tony Smith.\n");
}
