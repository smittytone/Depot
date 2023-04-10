/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano RP2040 Connect
 *
 * @version     1.2.2
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"


/*
 * STATIC PROTOTYPES
 */
static void nina_set_pin_mode(uint8_t pin, uint8_t mode);
static void nina_send_cmd(uint8_t cmd, uint8_t pin, uint8_t value);
static void nina_analog_write(uint8_t pin, uint8_t value);
static void nina_wait_for_ready(void);
static void spi_send_cmd(uint8_t cmd, uint8_t num_params);
static void spi_send_param(uint8_t* param, uint8_t param_len, bool is_last_param);
static inline uint32_t spi_transfer(uint8_t data);
static unsigned int spi_wait_response_cmd(uint8_t cmd, uint8_t numParam, uint8_t* param, uint8_t* param_len);


/*
 * GLOBALS
 */
Nano_LED_colour colour;


/**
 * @brief Initialise the Nina module and its SPI link
 */
void nano_led_init(void) {

    // Set Nina GPIO pins
    gpio_init(NINA_PIN_RSTN);
    gpio_set_dir(NINA_PIN_RSTN, GPIO_OUT);
    gpio_put(NINA_PIN_RSTN, false);

    gpio_init(NINA_PIN_READY);
    gpio_set_dir(NINA_PIN_READY, GPIO_IN);

    gpio_init(NINA_PIN_GPIO0);
    gpio_set_dir(NINA_PIN_GPIO0, GPIO_OUT);
    gpio_put(NINA_PIN_GPIO0, true);

    gpio_init(NINA_PIN_SPI_CS);
    gpio_set_dir(NINA_PIN_SPI_CS, GPIO_OUT);
    gpio_put(NINA_PIN_SPI_CS, true);

    gpio_put(NINA_PIN_RSTN, false);
    sleep_ms(10);
    gpio_put(NINA_PIN_RSTN, true);
    sleep_ms(750);
    gpio_put(NINA_PIN_GPIO0, false);
    gpio_set_dir(NINA_PIN_GPIO0, GPIO_IN);

    // Init SPI1
    spi_init(NINA_SPI, 8000000);
    spi_set_format(NINA_SPI, 8, 0, 0, SPI_MSB_FIRST);

    gpio_set_function(NINA_PIN_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(NINA_PIN_SPI_RX, GPIO_FUNC_SPI);
    gpio_set_function(NINA_PIN_SPI_SCK, GPIO_FUNC_SPI);

    // Tell Nina to configure these pins
    nina_set_pin_mode(NINA_LED_R, NINA_OUT);
    nina_set_pin_mode(NINA_LED_G, NINA_OUT);
    nina_set_pin_mode(NINA_LED_B, NINA_OUT);
}


/**
 * @brief Turn the LED off.
 */
void nano_led_off(void) {

    // SET Nina/ESP32 GPIO pins to HIGH
    nina_analog_write(NINA_LED_R, 255);
    nina_analog_write(NINA_LED_G, 255);
    nina_analog_write(NINA_LED_B, 255);
}


/**
 * @brief Turn the LED on.
 */
void nano_led_on(void) {

    // SET Nina/ESP32 GPIO pins to LOW
    nina_analog_write(NINA_LED_R, 255 - colour.red);
    nina_analog_write(NINA_LED_G, 255 - colour.green);
    nina_analog_write(NINA_LED_B, 255 - colour.blue);
}


/**
 * @brief Specify the LED's state (on or off).
 *
 * @param is_on: Turn the LED on (`true`) or off (`false`).
 */
void nano_led_set_state(bool is_on) {

    if (is_on) {
        nano_led_on();
    } else {
        nano_led_off();
    }
}


/**
 * @brief Flash the LED for the specified number of times.
 *
 * @param count: The number of blinks.
 */
void nano_led_flash(uint32_t count) {

     while (count > 0) {
        nano_led_on();
        sleep_ms(200);
        nano_led_off();
        sleep_ms(200);
        count--;
    }
}


/**
 * @brief Set the LED's colour, converting from a 24-bit RGB value
 *        to invidual 8-bit primary colour values.
 *        NOTE Sets the stored colour, but does not update the LED
 *             immediately. Call `nano_led_on()` to do so.
 *
 * @param rgb_colour: The colour as an RGB value bitfield:
 *                    R = bits 23-16, G = bits 15-8, B = bits 7-0.
 */
void nano_led_set_colour(uint32_t rgb_colour) {

    colour.blue = (rgb_colour & 0xFF);
    colour.green = ((rgb_colour & 0xFF00) >> 8);
    colour.red = ((rgb_colour & 0xFF0000) >> 16);
}


static void nina_set_pin_mode(uint8_t pin, uint8_t mode) {

    nina_send_cmd(NINA_CMD_SET_PIN_MODE, pin, mode);
}


static void nina_analog_write(uint8_t pin, uint8_t value) {

    nina_send_cmd(NINA_CMD_ANALOG_WRITE, pin, value);
}


static void nina_send_cmd(uint8_t cmd, uint8_t pin, uint8_t value) {

    // Select CS
    nina_wait_for_ready();
    gpio_put(NINA_PIN_SPI_CS, false);

    // Send Command
    spi_send_cmd(cmd, 2);
    spi_send_param((uint8_t*)&pin, 1, false);
    spi_send_param((uint8_t*)&value, 1, true);

    // Pad to multiple of 4
    spi_transfer(0xFF);  // 'D' for Dummy

    // Deselect CS
    gpio_put(NINA_PIN_SPI_CS, true);
}


static void nina_wait_for_ready(void) {

    uint64_t last = time_us_64();
    while (time_us_64() - last > 5000) {
        if (gpio_get(NINA_PIN_READY)) {
            break;
        }
    }
}


static void spi_send_cmd(uint8_t cmd, uint8_t num_params) {

    // Send SPI START CMD
    spi_transfer(NINA_CMD_START);

    // Send SPI C + cmd
    spi_transfer(cmd & ~0x80);

    // Send SPI numParam
    spi_transfer(num_params);

    // No params? Send END command
    if (num_params == 0) spi_transfer(NINA_CMD_END);
}


static void spi_send_param(uint8_t* param, uint8_t param_len, bool is_last_param) {

    // Send SPI paramLen
    spi_transfer(param_len);

    // Send SPI param data
    for (uint32_t i = 0 ; i < param_len ; ++i) {
        spi_transfer(param[i]);
    }

    // All done? Send END command
    if (is_last_param) spi_transfer(NINA_CMD_END);
}


static inline uint32_t spi_transfer(uint8_t data) {

    return spi_write_blocking(NINA_SPI, &data, 1);
}


/*
static unsigned int spi_wait_response_cmd(uint8_t cmd, uint8_t numParam, uint8_t* param, uint8_t* param_len) {

    char _data = 0;
    int ii = 0;

    IF_CHECK_START_CMD(_data) {
        CHECK_DATA(cmd | REPLY_FLAG, _data){};
        CHECK_DATA(numParam, _data) {
            readParamLen8(param_len);
            for (ii = 0 ; ii < (*param_len) ; ++ii)
            {
                // Get Params data
                getParam(&param[ii]);
            }
        }

        readAndCheckChar(END_CMD, &_data);
    }

    return 1;
}
*/
