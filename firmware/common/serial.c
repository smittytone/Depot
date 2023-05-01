/*
 * Depot RP2040 Bus Host Firmware - Primary serial and command functions
 *
 * @version     1.2.3
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "serial.h"


/*
 * STATIC PROTOTYPES
 */
// FROM 1.1.0
static void         sig_handler(int signal);
// FROM 1.1.2 -- make ack and err sends inline
static inline void  send_ack(void);
static inline void  send_err(void);
static uint32_t     rx(uint8_t *buffer);
// FROM 1.1.3
static void         set_mode(char mode_key);


/*
 * GLOBALS
 */
// FROM 1.1.0
// Access individual boards' pin arrays
extern uint8_t I2C_PIN_PAIRS_BUS_0[];
extern uint8_t I2C_PIN_PAIRS_BUS_1[];

// FROM 1.2.0
char supported_modes[MAX_NUMBER_OF_MODES] = { MODE_CODE_NONE };
I2C_State i2c_state;
OneWireState ow_state;
GPIO_State gpio_state;
// FROM 1.2.3
Button_State btn_state;


/**
 * @brief Listen on the USB-fed stdin for signals from the driver.
 */
void rx_loop(void) {

#ifdef DO_UART_DEBUG
    debug_init();
#endif

    // Trap certain signals
    signal(SIGABRT | SIGSEGV | SIGBUS | SIGTRAP | SIGSYS, sig_handler);

    // Prepare a UART RX buffer
    uint8_t rx_buffer[RX_BUFFER_LENGTH_B] = {0};
    uint32_t read_count = 0;
    bool do_use_led = true;

    // Prepare a transaction record with default data
    i2c_state.is_started = false;                         // No transaction taking place
    i2c_state.is_ready = false;                           // I2C bus not yet initialised
    i2c_state.frequency = 400;                            // The bud frequency in use
    i2c_state.address = 0xFF;                             // The target I2C address
    i2c_state.bus = DEFAULT_I2C_BUS == 0 ? i2c0 : i2c1;   // The I2C bus to use
    i2c_state.sda_pin = DEFAULT_SDA_PIN;                  // The I2C SDA pin
    i2c_state.scl_pin = DEFAULT_SCL_PIN;                  // The I2C SCL pin

    // FROM 1.1.0 -- record GPIO pin state
    memset(gpio_state.state_map, 0x00, GPIO_PIN_MAX + 1);

    // FROM 1.2.0 -- record OneWire state
    ow_state.is_ready = false;
    ow_state.data_pin = DEFAULT_DATA_PIN;
    ow_state.current_device = 0;
    ow_state.device_count = 0;

    // FROM 1.1.3
    // Default current mode to I2C, for backwards compatibility
    // NOTE Call the function so the LED colour is correctly set
    uint8_t current_mode = MODE_CODE_I2C;
    supported_modes[0] = MODE_CODE_I2C;
    supported_modes[1] = MODE_CODE_ONE_WIRE;
    set_mode(MODE_CODE_I2C);

    // FROM 1.1.3
    uint last_error_code = GEN_NO_ERROR;

    // FROM 1.2.3
    // Button variables
    memset(btn_state.buttons, 0x00, sizeof(Button*) * 32);
    btn_state.state = 0;
    btn_state.count = 0;

    // Heartbeat variables
    uint64_t last = time_us_64();
    bool is_on = false;

    while(1) {
        // Scan for input
        read_count = rx(rx_buffer);

        // Did we receive anything?
        if (read_count > 0) {
            // Are we expecting write data or a read op next?
            // NOTE The first byte will always be:
            //      32-127  (ascii char as a command),
            //      128-191 (read 1-64 bytes), or
            //      192-255 (write 1-64 bytes)
            uint8_t status_byte = rx_buffer[0];

            if (status_byte >= READ_LENGTH_BASE) {
                // We have data or a read op
                if (status_byte >= WRITE_LENGTH_BASE) {
                    // Write data received, so send it and ACK
                    switch(current_mode){
                        case MODE_CODE_I2C:
                            if (i2c_state.is_started) {
                                i2c_state.write_byte_count = status_byte - WRITE_LENGTH_BASE + 1;
#ifdef DO_UART_DEBUG
                                debug_log("Bytes to write: %i", i2c_state.write_byte_count);
#endif
                                int bytes_sent = i2c_write_timeout_us(i2c_state.bus, i2c_state.address, &rx_buffer[1], i2c_state.write_byte_count, false, 1000);
#ifdef DO_UART_DEBUG
                                debug_log("Bytes sent: %i", bytes_sent);
#endif
                                // Send an ACK to say we wrote the data -- or an ERR if we didn't
                                if (bytes_sent != PICO_ERROR_GENERIC && bytes_sent != PICO_ERROR_TIMEOUT) {
                                    send_ack();
                                    break;
                                }
                            }

                            // Error
                            last_error_code = I2C_COULD_NOT_WRITE;
                            send_err();
                            break;
                        case MODE_CODE_ONE_WIRE:
                            if (ow_state.is_ready) {
                                ow_state.write_byte_count = status_byte - WRITE_LENGTH_BASE + 1;
#ifdef DO_UART_DEBUG
                                debug_log("Bytes to write: %i", ow_state.write_byte_count);
#endif
                                for (uint32_t i = 0 ; i < ow_state.write_byte_count ; ++i) {
                                    ow_write_byte(&ow_state, rx_buffer[i + 1]);
#ifdef DO_UART_DEBUG
                                debug_log("Written: %02X", rx_buffer[i + 1]);
#endif
                                }

                                send_ack();
                            } else {
                                last_error_code = OW_NOT_READY;
                                send_err();
                            }
                            break;
                        default:
                            last_error_code = GEN_UNKNOWN_MODE;
                            send_err();
                    }
                } else {
                    // Read length received only
                    uint8_t bus_rx_buffer[BUS_RX_BUFFER_LENGTH_B] = {0};
                    switch(current_mode){
                        case MODE_CODE_I2C:
                            if (i2c_state.is_started) {
                                i2c_state.read_byte_count = status_byte - READ_LENGTH_BASE + 1;

                                int bytes_read = i2c_read_timeout_us(i2c_state.bus, i2c_state.address, bus_rx_buffer, i2c_state.read_byte_count, false, 1000);

                                // Return the read data
                                if (bytes_read != PICO_ERROR_GENERIC) {
                                    tx(bus_rx_buffer, i2c_state.read_byte_count);
                                    break;
                                }
                            }
                            last_error_code = I2C_COULD_NOT_READ;
                            break;
                        case MODE_CODE_ONE_WIRE:
                            if (ow_state.is_ready) {
                                ow_state.read_byte_count = status_byte - READ_LENGTH_BASE + 1;

                                for (uint32_t i = 0 ; i < ow_state.read_byte_count ; ++i) {
                                    bus_rx_buffer[i] = ow_read_byte(&ow_state);
#ifdef DO_UART_DEBUG
                                    debug_log("Read: %02X", bus_rx_buffer[i]);
#endif
                                }

                                tx(bus_rx_buffer, ow_state.read_byte_count);
                            }
                            break;
                        default:
                            last_error_code = GEN_UNKNOWN_MODE;
                            send_err();
                    }
                }
            } else {
                // Maybe we received a command
                char cmd = (char)status_byte;

#ifdef DO_UART_DEBUG
                debug_log("Command received: %c 0x%02X", cmd, status_byte);
#endif

                switch(cmd) {
                    /*
                     * FIRMWARE COMMANDS
                     */

                    // FROM 1.1.1 -- change command from z to !
                    case 'z':   // REMOVE IN 1.2.0
                    case '!':   // RESPOND TO CONNECTION REQUEST
                        // FROM 1.2.0
                        // Replace the 'hello' string with backwards compatible
                        // data that includes a firmware version indicator.
                        // This saves code an explicit request for info, which in
                        // any case is intended to be human-readable only.
                        // This will be useful for future apps to detect which
                        // firmware they're talking to.
                        // TODO Automate from cmake
                        tx("OK12", 4);
                        break;

                    // FROM 1.1.0
                    case '*':   // SET LED STATE
                        do_use_led = rx_buffer[1] == 1 ? true : false;
#ifdef SHOW_HEARTBEAT
                        send_ack();
#else
                        last_error_code = GEN_LED_NOT_ENABLED;
                        send_err();
#endif
                        break;

                    case '?':   // GET STATUS
                        switch(current_mode) {
                            case MODE_CODE_I2C:
                                send_i2c_status(&i2c_state);
                                break;
                            case MODE_CODE_ONE_WIRE:
                                ow_send_state(&ow_state);
                                break;
                            default:
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                        }
                        break;

                    // FROM 1.1.3
                    case '$':   // GET LAST ERROR
                        {
                            uint8_t err_buffer[3] = {(uint8_t)last_error_code, '\r', '\n'};
                            tx(err_buffer, 3);
#ifdef DO_UART_DEBUG
                            debug_log("Error code reported: %02X", last_error_code);
#endif
                        }
                        break;

                    // FROM 1.2.0
                    case '#':   // SET CURRENT MODE
                        {
                            char new_mode = rx_buffer[1];
                            bool is_mode_supported = false;
                            for (uint32_t i = 0 ; i < MAX_NUMBER_OF_MODES ; ++i) {
                                if (supported_modes[i] != MODE_CODE_NONE && supported_modes[i] == new_mode) {
                                    is_mode_supported = true;
                                    break;
                                }
                            }

                            if (is_mode_supported) {
                                current_mode = new_mode;
                                set_mode(new_mode);
                                send_ack();
#ifdef DO_UART_DEBUG
                                debug_log("Mode set to: %02X", current_mode);
#endif
                            } else {
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                            }

                            break;
                        }
                    /*
                     * MULTI-BUS COMMANDS
                     */

                    // FROM 1.1.0
                    case 'c':   // CONFIGURE THE BUS AND PINS
                        {
                            bool success = false;
                            uint32_t possible_error = GEN_NO_ERROR;
                            switch(current_mode) {
                                case MODE_CODE_I2C:
                                    success = configure_i2c(&i2c_state, &rx_buffer[1]);
                                    possible_error = I2C_COULD_NOT_CONFIGURE;
                                    break;
                                case MODE_CODE_ONE_WIRE:
                                    success = ow_configure(&ow_state, rx_buffer[1]);
                                    possible_error = OW_COULD_NOT_CONFIGURE;
                                    break;
                                default:
                                    last_error_code = GEN_UNKNOWN_MODE;
                                    send_err();
                            }

                            if (success) {
                                send_ack();
                            } else {
                                last_error_code = possible_error;
                                send_err();
                            }
                        }
                        break;

                    case 'd':   // SCAN THE CURRENT BUS FOR DEVICES
                                // BUSES SUPPORTED: I2C, ONE-WIRE
                        switch(current_mode) {
                            case MODE_CODE_I2C:
                                if (!i2c_state.is_ready) init_i2c(&i2c_state);
                                send_i2c_scan(&i2c_state);
                                break;
                            case MODE_CODE_ONE_WIRE:
                                ow_send_scan(&ow_state);
                                break;
                            default:
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                        }
                        break;

                    case 'i':   // INITIALISE THE CURRENT BUS:
                                // BUSES SUPPORTED: I2C, ONE-WIRE
                        switch(current_mode) {
                            case MODE_CODE_I2C:
                                // No need it initialise if we already have
                                if (!i2c_state.is_ready) {
                                    // Are the pins already taken?
                                    if ((is_pin_taken(i2c_state.scl_pin) & ~PIN_USAGE_FIELD_I2C) > 0 ||
                                        (is_pin_taken(i2c_state.sda_pin) & ~PIN_USAGE_FIELD_I2C) > 0) {
                                        last_error_code = I2C_PINS_ALREADY_IN_USE;
                                        send_err();
                                        break;
                                    }

                                    // Initialise the bus
                                    init_i2c(&i2c_state);
                                }
                                send_ack();
                                break;
                            case MODE_CODE_ONE_WIRE:
                                // Is the data pin already taken?
                                if ((is_pin_taken(ow_state.data_pin) & ~PIN_USAGE_FIELD_ONEWIRE) > 0) {
                                    last_error_code = OW_PIN_ALREADY_IN_USE;
                                    send_err();
                                    break;
                                }

                                // Initialise the bus
                                ow_init(&ow_state);
                                if (ow_state.is_ready) {
                                    send_ack();
                                } else {
                                    last_error_code = OW_NO_DEVICES_FOUND;
                                    send_err();
                                }
                                break;
                            default:
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                        }
                        break;

                    case 'x':   // RESET BUS
                        switch(current_mode) {
                            case MODE_CODE_I2C:
                                i2c_state.is_started = false;
                                reset_i2c(&i2c_state);
                                send_ack();
                                break;
                            case MODE_CODE_ONE_WIRE:
                                ow_reset(&ow_state);
                                send_ack();
                                break;
                            default:
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                        }
                        break;

                    // FROM 1.1.3
                    case 'k':   // DEINIT BUS
                        switch(current_mode) {
                            case MODE_CODE_I2C:
                                deinit_i2c(&i2c_state);
                                send_ack();
                                break;
                            default:
                                last_error_code = GEN_UNKNOWN_MODE;
                                send_err();
                        }
                        break;

                    /*
                     * I2C-SPECIFIC COMMANDS
                     */
                    case '1':   // SET BUS TO 100kHz
                        set_i2c_frequency(&i2c_state, 100);
                        send_ack();
                        break;

                    case '4':   // SET BUS TO 400kHZ
                        set_i2c_frequency(&i2c_state, 400);
                        send_ack();
                        break;

                    case 'p':   // SEND AN I2C STOP
                        if (i2c_state.is_ready && i2c_state.is_started) {
                            // Send no bytes and STOP
                            uint8_t data = 0;
                            i2c_write_timeout_us(i2c_state.bus, i2c_state.address, &data, 1, false, 1000);

                            // Reset state
                            i2c_state.is_started = false;
                            i2c_state.is_read_op = false;
                            send_ack();
                        } else {
                            last_error_code = I2C_ALREADY_STOPPED;
                            send_err();
                        }
                        break;

                    case 's':   // START AN I2C TRANSACTION
                        if (i2c_state.is_ready) {
                            // Received data is in the form ['s', (address << 1) | op];
                            i2c_state.address = (rx_buffer[1] & 0xFE) >> 1;
                            i2c_state.is_read_op = ((rx_buffer[1] & 0x01) == 1);
                            i2c_state.is_started = true;
                            send_ack();
                        } else {
                            last_error_code = I2C_NOT_READY;
                            send_err();
                        }
                        break;

                    /*
                     * ONE-WIRE COMMANDS
                     */


                    // FROM 1.2.0

                    /*
                     * GPIO COMMANDS
                     */

                    // FROM 1.1.0
                    case 'g':   // SET DIGITAL OUT PIN
                        {
                            uint8_t read_value = 0;
                            uint8_t gpio_pin = (rx_buffer[1] & 0x1F);
                            bool is_read = (rx_buffer[1] & 0x20);

                            // Make sure the pin's not in use by a bus
                            if (is_pin_taken(gpio_pin) > 1) {
                                last_error_code = GPIO_PIN_ALREADY_IN_USE;
                                send_err();
                                break;
                            }

                            // FROM 1.2.0
                            // Clear the pin? Check for a postfix byte of the right value
                            if (read_count > 2 && (rx_buffer[2] & 0x80)) {
                                clear_pin(&gpio_state, gpio_pin);
                                send_ack();
                                break;
                            }

                            if (!set_gpio(&gpio_state, &read_value, rx_buffer)) {
                                last_error_code = GPIO_CANT_SET_PIN;
                                send_err();
                                break;
                            }

                            putchar(is_read ? read_value : 0xFF);
                        }
                        break;

                    case 'b':   // SET BUTTON ON GPIO
                        {
                            uint8_t gpio_pin = (rx_buffer[1] & 0x1F);
                            bool is_read = (rx_buffer[1] & 0x20);

                            // Read operation? Return the four-byte status
                            if (is_read) {
                                // TODO CHECK BYTE ORDER AT RECEIVE END!!!!
                                tx((uint8_t*)&btn_state.state, 4);
                                break;
                            }

                            // Clear operation? Check for a postfix byte of the right value
                            if (read_count > 2 && (rx_buffer[2] & 0x80)) {
                                clear_button(&btn_state, gpio_pin);
                                send_ack();
                                break;
                            }

                            // If we've got this far, this is a set operation.
                            // Make sure the button's GPIO pin is good to use.
                            if (is_pin_taken(gpio_pin) > 1) {
                                last_error_code = GPIO_PIN_ALREADY_IN_USE;
                                send_err();
                                break;
                            }

                            // Attempt to set the button
                            if (!set_button(&btn_state, rx_buffer)) {
                                last_error_code = GPIO_CANT_SET_BUTTON;
                                send_err();
                                break;
                            }

                            send_ack();
                        }
                        break;

                    default:    // UNKNOWN COMMAND -- FAIL
                        last_error_code = GEN_UNKNOWN_COMMAND;
                        send_err();
                }
            }

            // Clear buffer and listen for input
            memset(rx_buffer, 0, read_count);
        }

#ifdef SHOW_HEARTBEAT
        // Heartbeat LED blink for debugging
        if (do_use_led) {
            uint64_t now = time_us_64();
            if (now - last > HEARTBEAT_PERIOD_US) {
                led_set_state(true);
                is_on = true;
                last = now;

#ifdef DO_UART_DEBUG
                debug_log("LED ON");
#endif

            } else if ((now - last > HEARTBEAT_FLASH_US) && is_on) {
                led_set_state(false);
                is_on = false;

#ifdef DO_UART_DEBUG
                debug_log("LED OFF");
#endif

            }
        }
#endif

        // FROM 1.2.3
        // Button checks
        if (btn_state.count > 0) poll_buttons(&btn_state);

        // Pause? May not be necessary or might be bad
        sleep_ms(RX_LOOP_DELAY_MS);
    }

    // Should not get here, but just in case...
    // Signal an error on the host's LED
    led_set_colour(0xFF0000);
    led_on();

    // Fall out of the firmware at this point...
}


/**
 * @brief Send a single-byte ACK.
 */
static inline void send_ack(void) {
#ifdef BUILD_FOR_TERMINAL_TESTING
    printf("ACK\r\n");
#else
    putchar(ACK);
#ifdef DO_UART_DEBUG
    debug_log("********** ACK **********");
#endif
#endif
}


/**
 * @brief Send a single-byte ERR.
 */
static inline void send_err(void) {
#ifdef BUILD_FOR_TERMINAL_TESTING
    printf("ERR\r\n");
#else
    putchar(ERR);
#endif
}


/**
 * @brief Read in a single transmitted block.
 *
 * @param buffer: A pointer to the byte store buffer.
 *
 * @returns The number of bytes to process.
 */
static uint32_t rx(uint8_t* buffer) {

    uint32_t buffer_byte_count = 0;
    int c = PICO_ERROR_TIMEOUT;
    while (buffer_byte_count < RX_BUFFER_LENGTH_B) {
        c = getchar_timeout_us(1);
        if (c == PICO_ERROR_TIMEOUT) break;
        buffer[buffer_byte_count++] = (uint8_t)c;
        // FROM 1.2.3 -- remove the delay
        //sleep_ms(UART_LOOP_DELAY_MS);
    }

#ifdef DO_UART_DEBUG
    if (buffer_byte_count > 0) debug_log("Bytes received: %i", buffer_byte_count);
#endif
    return buffer_byte_count;
}


/**
 * @brief Send a single transmitted block.
 *
 * @param buffer:     A pointer to the byte store buffer.
 * @param byte_count: The number of bytes to send.
 */
void tx(uint8_t* buffer, uint32_t byte_count) {

    for (uint32_t i = 0 ; i < byte_count ; ++i) {
        putchar((buffer[i]));
        // FROM 1.2.3 -- remove the delay
        //sleep_ms(UART_LOOP_DELAY_MS);
    }
}


/**
 * @brief Return in the mode (I2C, SPI, etc.) integer ID from the
 *        char ID sent to the host from the client.
 *
 *        It also sets the device LED colour.
 *
 * @param mode_key: The mode character: `i`, `s` etc.
 */
static void set_mode(char mode_key) {

    switch(mode_key) {
        case MODE_CODE_I2C:
            led_set_colour(COLOUR_MODE_I2C);
            break;
        case MODE_CODE_SPI:
            led_set_colour(COLOUR_MODE_SPI);
            break;
        case MODE_CODE_UART:
            led_set_colour(COLOUR_MODE_UART);
            break;
        case MODE_CODE_ONE_WIRE:
        case MODE_CODE_ONE_WIRE_ALT:
            led_set_colour(COLOUR_MODE_ONE_WIRE);
            break;
        default:
            led_set_colour(COLOUR_MODE_ONE_NONE);
            mode_key = MODE_CODE_NONE;
    }

#ifdef DO_UART_DEBUG
    debug_log("Mode set: %c", mode_key);
#endif
}


static void sig_handler(int signal) {

#ifdef DO_UART_DEBUG
    debug_log("Signal received: %i", signal);
#endif

}


/**
 * @brief Check whether a given pin is in use by any bus or gpio.
 *
 * @param pin: The pin number.
 *
 * @returns A bitfield indicating usage:
 *          Bit 0 - GPIO
 *              1 - I2C
 *              5 - 1-Wire
 *          All other bits reserved for future use.
 */
uint8_t is_pin_taken(uint32_t pin) {

    uint8_t bitfield = is_pin_in_use_by_gpio(&gpio_state, pin) ? PIN_USAGE_FIELD_GPIO : 0;
    bitfield |= is_pin_in_use_by_i2c(&i2c_state, pin) ? PIN_USAGE_FIELD_I2C : 0;
    bitfield |= is_pin_in_use_by_ow(&ow_state, pin) ? PIN_USAGE_FIELD_ONEWIRE : 0;
    return bitfield;
}
