/*
 * macOS/Linux 1-Wire CLI utility
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Static Prototypes

static int          process_commands(SerialDriver *sd, int argc, char *argv[], uint32_t delta);
static inline void  show_help(void);
static inline void  show_version(void);
static inline void  show_commands(void);
static inline void  show_bad_command_help(char* command);


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
        fprintf(stderr, "Usage: cliwire {DEVICE_PATH} [command] ... [command]\n");
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
                    fprintf(stderr, "cliwire requires a board with firmware 1.2.0 or above... exiting\n");
                    return EXIT_ERR;
                }

                // Set the mode to 1-Wire
                if (!serial_set_mode(&board, MODE_CODE_ONE_WIRE)) {
                    serial_flush_and_close_port(&board);
                    fprintf(stderr, "Could not set board mode... exiting\n");
                    return EXIT_ERR;
                }

                // Process the remaining commands in sequence
                int result = process_commands(&board, argc, argv, delta);
                serial_flush_and_close_port(&board);
                return result;
            }
        } else {
            fprintf(stderr, "No commands supplied... exiting\n");
            return EXIT_OK;
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

    fprintf(stderr, "cliwire {device} [commands]\n\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  {device} is a mandatory device path, eg. /dev/cu.usbmodem-101.\n");
    fprintf(stderr, "  [commands] are optional commands, as shown below.\n\n");
    show_commands();
}


/**
 * @brief Show app version.
 */
static inline void show_version(void) {

    fprintf(stderr, "cliwire %s\n", APP_VERSION);
    fprintf(stderr, "Copyright © 2023, Tony Smith.\n");
}


/**
 * @brief Output help info.
 */
static inline void show_commands(void) {

    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  z                                Initialise 1-Wire.\n");
    fprintf(stderr, "  c {bus ID} {SDA pin} {SCL pin}   Configure 1-Wire.\n");
    fprintf(stderr, "  r {address} {count}              Read count bytes in from 1-Wire.\n");
    fprintf(stderr, "  s                                Scan for devices on the 1-Wire host.\n");
    fprintf(stderr, "  i                                Get 1-Wire host device information.\n");
    fprintf(stderr, "  l {on|off}                       Turn the 1-Wire host LED on or off.\n");
    fprintf(stderr, "  h                                Show help and quit.\n");
}


/**
 * @brief Output help info on receipt of a bad command.
 *
 * @param command: The bad command.
 */
static inline void show_bad_command_help(char* command) {

    print_error("Bad command: %s\n", command);
}


#pragma mark - Command Parsing and Processing

/**
 * @brief Parse driver commands.
 *
 * @param sd:    Pointer to a SerialDriver structure.
 * @param argc:  The max number of args to process.
 * @param argv:  The args.
 * @param delta: An offset to the first board command arg.
 *
 * @returns The driver exit code, 0 on success, 1 on failure.
 */
static int process_commands(SerialDriver *sd, int argc, char *argv[], uint32_t delta) {

    // Set a 10ms period for intra-command delay period
    struct timespec pause;
    pause.tv_sec = 0.010;
    pause.tv_nsec = 0.010 * 1000000;

    // Process args one by one
    for (int i = delta ; i < argc ; i++) {
        char* command = argv[i];

#ifdef DEBUG
        print_log("Command: %s", command);
#endif

        // Commands should be single characters
        if (strlen(command) != 1) {
            // FROM 1.1.0 -- Allow for commands with a - prefix
            if (command[0] == '-') {
                command++;
            } else {
                show_bad_command_help(command);
                return EXIT_ERR;
            }
        }

        switch (command[0]) {
            case 'C':
            case 'c':   // CHOOSE 1-WIRE DATA PIN
                {
                    if (i < argc - 1) {
                        char* token = argv[++i];
                        long data_pin = strtol(token, NULL, 0);

                        // Make sure we have broadly valid pin numbers
                        if (data_pin < 0 || data_pin > 32) {
                            print_error("Unsupported pin value specified");
                            return EXIT_ERR;
                        }

#if DEBUG
                        printf("1-Wire data pin %li\n", data_pin);
#endif

                        bool result = one_wire_configure_bus(sd, data_pin);
                        if (!result) print_warning("1-Wire bus config un-ACK’d");
                        break;
                    }

                    print_error("Incomplete 1-Wire setup data given");
                    return EXIT_ERR;
                }

            case 'E':
            case 'e':   // PRINT LAST BOARD ERROR
                serial_get_last_error(sd);
                break;

            case 'G':   // FROM 1.1.0
            case 'g':   // SET OR GET A GPIO PIN
                {
                    if (i < argc - 1) {
                        char* token = argv[++i];
                        long pin_number = strtol(token, NULL, 0);

                        if (pin_number < 0 || pin_number > 31) {
                            print_error("Pin out of range (0-31");
                            return EXIT_ERR;
                        }

                        if (i < argc - 1) {
                            token = argv[++i];
                            // Is this a read op?
                            bool do_read   = (token[0] == 'r' || token[0] == 'R');

                            // Is it a state change?
                            bool pin_state = (token[0] == '1');
                            bool want_high = (strncasecmp(token, "hi", 2) == 0);
                            bool want_low  = (strncasecmp(token, "lo", 2) == 0);
                            if (want_high || want_low) pin_state = want_high || !want_low;

                            // Pin direction is optional
                            bool pin_direction = true;
                            if (i < argc - 1) {
                                token = argv[++i];
                                if (token[0] == '0' || token[0] == '1') {
                                    pin_direction = (token[0] == '1');
                                } else if (token[0] == 'i' || token[0] == 'o') {
                                    bool dir_in  = (strcasecmp(token, "in") == 0);
                                    bool dir_out = (strcasecmp(token, "out") == 0);
                                    if (dir_in || dir_out) pin_direction = dir_out || !dir_in;
                                } else {
                                    i -= 1;
                                }
                            }

                            // Encode the TX data:
                            // Bit 7 6 5 4 3 2 1 0
                            //     | | | |_______|________ Pin number 0-31
                            //     | | |__________________ Read flag (1 = read op)
                            //     | |____________________ Direction bit (1 = out, 0 = in)
                            //     |______________________ State bit (1 = HIGH, 0 = LOW)

                            uint8_t send_byte = (uint8_t)pin_number;
                            send_byte &= 0x1F;
                            if (pin_state) send_byte |= 0x80;
                            if (pin_direction) send_byte |= 0x40;
                            if (do_read) send_byte |= 0x20;

                            if (do_read) {
                                // Read back the pin value
                                uint8_t result = gpio_get_pin(sd, send_byte);

                                // Issue value to STDOUT
                                fprintf(stdout, "%02X\n", ((result & 0x80) >> 7));

                                // Check we got the same pin back that we asked for
                                if ((result & 0x1F) != pin_number) print_warning("GPIO pin set un-ACK’d");
                            } else {
                                // Set the pin and wait for ACK
                                bool result = gpio_set_pin(sd, send_byte);
                                if (!result) print_warning("GPIO pin set un-ACK’d");
                            }
                            break;
                        }

                        print_error("No state value given");
                        return EXIT_ERR;
                    }

                    print_error("No pin value given");
                    return EXIT_ERR;
                }

            case 'I':
            case 'i':   // PRINT HOST STATUS INFO
                one_wire_get_info(sd, true);
                break;

            case 'L':
            case 'l':   // SET THE BOARD LED
                {
                    // Get the state if we can
                    if (i < argc - 1) {
                        char* token = argv[++i];
                        bool is_on = (strcasecmp(token, "on") == 0);
                        if (is_on || strcasecmp(token, "off") == 0 ) {
                            bool result = serial_set_led(sd, is_on);
                            if (!result) print_warning("LED set un-ACK'd");
                            break;
                        }

                        print_error("Invalid LED state give");
                        return EXIT_ERR;
                    }

                    print_error("No LED state given");
                    return EXIT_ERR;
                }

            case 'R':
            case 'r':   // READ FROM THE BUS
                {
                    // Get the number of bytes if we can
                    if (i < argc - 1) {
                        char* token = argv[++i];
                        size_t num_bytes = strtol(token, NULL, 0);
                        uint8_t bytes[4096];

                        one_wire_read_bytes(sd, bytes, num_bytes);
                        break;
                    } else {
                        print_error("No byte total given");
                    }

                    return EXIT_ERR;
                }

            case 'S':
            case 's':   // LIST DEVICES ON BUS
                one_wire_scan(sd);
                break;

            case 'W':
            case 'w':   // WRITE TO THE 1-Wire BUS
                {
                    // Get the bytes to write if we can
                    if (i < argc - 1) {
                        char* token = argv[++i];
                        size_t num_bytes = 0;
                        uint8_t bytes[1024];
                        char* endptr = token;

                        while (num_bytes < sizeof(bytes)) {
                            bytes[num_bytes++] = (uint8_t)strtol(endptr, &endptr, 0);
                            if (*endptr == '\0') break;
                            if (*endptr != ',') {
                                print_error("Invalid bytes: %s\n", token);
                                return EXIT_ERR;
                            }

                            endptr++;
                        }

                        one_wire_write_bytes(sd, bytes, num_bytes);
                        break;
                    }

                    print_error("No bytes given");
                    return EXIT_ERR;
                }

            case 'X':
            case 'x':
                one_wire_reset(sd);
                break;

            case 'Z':
            case 'z':   // INITIALISE BUS
                // Initialize the board's 1-Wire line
                if (!(one_wire_init(sd))) {
                    print_error("Could not initialise 1-Wire");
                    return EXIT_ERR;
                }

                break;

            default:    // NO COMMAND/UNKNOWN COMMAND
                show_bad_command_help(command);
                return EXIT_ERR;
        }

        // Pause for the UART's breath
        nanosleep(&pause, &pause);
    }

    return 0;
}
