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
bool do_exit = false;

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
            // Don't output data we receive from the board
            // NOTE Requires Depot FW 1.2.3
            serial_output_read_data(false);
            
            // Set up delay timings
            struct timespec now, pause;
            pause.tv_sec = 0.020;
            pause.tv_nsec = 0.020 * 1000000;

            // Configure the buttons remaining commands in sequence
            Button* btn1 = (Button*)malloc(sizeof(Button));
            btn1->gpio = 1;
            btn1->set = false;
            btn1->pressed = false;
            btn1->trigger_on_release = false;
            buttons[button_count] = btn1;
            button_count++;
            
            Button* btn2 = (Button*)malloc(sizeof(Button));
            btn2->gpio = 2;
            btn2->set = false;
            btn2->pressed = false;
            btn2->trigger_on_release = true;
            buttons[button_count] = btn2;
            button_count++;
            
            // Configure the buttons' GPIO pins
            for (uint32_t i = 0 ; i < button_count ; ++i) {
                Button* btn = buttons[i];
                if (!gpio_set_pin(&board, (btn->gpio & 0x1F))) {
                    fprintf(stderr, "BUTTON %i CONF ERR\n", i);
                    exit(1);
                }
            }
            
            // Poll the buttons, one by one
            Button* btn;
            while(1) {
                for (uint32_t i = 0 ; i < button_count ; ++i) {
                    // Get the next button to poll
                    btn = buttons[i];
                    
                    // Set bit 5 for a read op
                    bool pin_high = (gpio_get_pin(&board, (btn->gpio | 0x20)) & 0x80);
                    
                    // Pin state is indicated by bit 7
                    if (!pin_high && btn->pressed) {
                        // BUTTON RELEASED
                        btn->pressed = false;
                        if (btn->trigger_on_release) perform_action(i);
                    } else if (pin_high) {
                        // BUTTON PRESSED?
                        if (!btn->set) {
                            // No press seen yet, so assume one and start the count
                            btn->set = true;
                            clock_gettime(CLOCK_MONOTONIC_RAW, &btn->press);
                        } else {
                            // Button has been pressed -- check count
                            if (!btn->pressed) {
                                clock_gettime(CLOCK_MONOTONIC_RAW, &now);
                                struct timespec then = btn->press;
                                if (now.tv_nsec - then.tv_nsec >= 10000000) {
                                    // Still held after debounce period
                                    btn->set = false;
                                    btn->pressed = true;
                                    
                                    if (!btn->trigger_on_release) perform_action(i);
                                }
                            }
                        }
                    }   
                }

                // Short ns delay
                nanosleep(&pause, &pause);
                
                // Was the exit button pressed?
                if (do_exit) {
                    for (uint32_t i = 0 ; i < button_count ; ++i) free(buttons[i]);
                    exit(EXIT_OK);
                }
            }   
        }
    }

    if (board.file_descriptor != -1) serial_flush_and_close_port(&board);
    return EXIT_ERR;
}


static void perform_action(uint32_t btn_number) {
    
    Button* btn = buttons[btn_number];
    if (btn->trigger_on_release) {
        fprintf(stderr, "BUTTON ON GPIO %i RELEASED\n", btn->gpio);
    } else {
        fprintf(stderr, "BUTTON ON GPIO %i PRESSED\n", btn->gpio);
    }
    
    switch(btn_number) {
        case 0:
            do_exit = true;
            break;
        case 1:
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
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  a [on|off]                      Activate/deactivate the display. Default: on.\n");
    fprintf(stderr, "  b {0-15}                        Set the display brightness from low (0) to high (15).\n");
    fprintf(stderr, "  h                               Help information.\n\n");
}


/**
 * @brief Show app version.
 */
static inline void show_version(void) {

    fprintf(stderr, "buttons %s\n", APP_VERSION);
    fprintf(stderr, "Copyright © 2023, Tony Smith.\n");
}
