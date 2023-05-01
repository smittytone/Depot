/*
 * Multiple button input
 *
 * Version 1.2.3
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
#include "main.h"


#pragma mark - Static Prototypes

static void         perform_action(uint32_t pin_number);
static void         show_help(void);
static inline void  show_version(void);


#pragma mark - Global Vars

// A serial comms structure
SerialDriver board;

Button* buttons[8];
uint32_t button_count = 0;

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
        fprintf(stderr, "Usage:  buttons {DEVICE_PATH}\n");
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
        serial_connect(&board, argv[1]);
        if (board.is_connected) {
            struct timespec now, then;

            // Configure the buttons remaining commands in sequence
            Button* btn = (Button*)malloc(sizeof(Button));
            btn->gpio = 21;
            btn->set = false;
            btn->pressed = false;
            btn->trigger_on_release = true;
            buttons[button_count] = btn;
            button_count++;

            for (uint32_t i = 0 ; i < button_count ; ++i) {
                btn = buttons[i];
                if (!gpio_set_pin(&board, btn->gpio)) {
                    // ERROR
                }
            }

            // Poll the buttons, one by one
            while(1) {
                for (uint32_t i = 0 ; i < button_count ; ++i) {
                    btn = buttons[i];
                    uint8_t pin_value = gpio_get_pin(&board, (btn->gpio & 0x20));

                    if (pin_value == 0 && btn->pressed) {
                        // BUTTON RELEASED
                        btn->pressed = false;
                        perform_action(i);
                    } else if (pin_value == 1) {
                        // BUTTON PRESSED?
                        if (!btn->set) {
                            // No press seen yet, so assume one and start the count
                            btn->set = true;
                            clock_gettime(CLOCK_MONOTONIC_RAW, btn->press);
                        } else {
                            // Button has been pressed -- check count
                            if (!btn->pressed) {
                                struct timespec now;
                                clock_gettime(CLOCK_MONOTONIC_RAW, &now);
                                if (now.tv_nsec - btn->press->tv_nsec >= 10000000) {
                                    // Still held after debounce period
                                    btn->set = false;
                                    
                                    if (btn->trigger_on_release) {
                                        btn->pressed = true;
                                    } else {
                                        perform_action(i);
                                    }
                                }
                            }
                        }
                    }   
                }

                // Some ms or us delay
            }   
        }
    }

    if (board.file_descriptor != -1) serial_flush_and_close_port(&board);
    return EXIT_ERR;
}


static void perform_action(uint32_t pin_number) {

    switch(pin_number) {
        case 0:
            fprintf(stderr, "BUTTON 0 PRESSED\n");
            break;
        default:
            break;
    }
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
