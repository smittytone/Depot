/*
 * Depot RP2040 Bus Host Firmware - Arduino Nano RP2040 Connect
 *
 * @version     1.3.0
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
static void nina_pin_write(uint8_t pin, uint8_t value);
static void nina_wait_for_ready(void);


/*
 * TODO
 *
 * Update the code here to use the Nano's RGB LED
 */


/*
 * GLOBALS
 */
RGB_LED_colour colour;


/**
 * @brief Initialise the Nina module and its SPI link
 */
void nano_led_init(void) {

    // Set Nina GPIO pins
    gpio_init(PIN_MONO_LED);
    gpio_set_dir(PIN_MONO_LED, GPIO_OUT);
    gpio_put(PIN_MONO_LED, false);

    /*
    gpio_init(NINA_PIN_RSTN);
    gpio_set_dir(NINA_PIN_RSTN, GPIO_OUT);
    gpio_put(NINA_PIN_RSTN, false);

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
    */
}


/**
 * @brief Turn the LED off.
 */
void nano_led_off(void) {

    // SET Nina/ESP32 GPIO pins to HIGH
    /*
    nina_pin_write(NINA_LED_R, 1);
    nina_pin_write(NINA_LED_G, 1);
    nina_pin_write(NINA_LED_B, 1);
    */
    
    // Use the mono LED for now
    gpio_put(PIN_MONO_LED, false);
}


/**
 * @brief Turn the LED on.
 */
void nano_led_on(void) {

    // SET Nina/ESP32 GPIO pins to LOW
    /*
    nina_pin_write(NINA_LED_R, 0);
    nina_pin_write(NINA_LED_G, 0);
    nina_pin_write(NINA_LED_B, 0);
    */

    // Use the mono LED for now
    gpio_put(PIN_MONO_LED, true);
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


static void nina_pin_write(uint8_t pin, uint8_t value) {

    nina_send_cmd(NINA_CMD_ANALOG_WRITE, pin, value);
}


static void nina_send_cmd(uint8_t cmd, uint8_t pin, uint8_t value) {

    uint8_t buffer[8] = {0};
    buffer[0] = NINA_CMD_START;     // Packet start
    buffer[1] = cmd & 0x7F;         // Command + reply bit (7)  
    buffer[2] = 2;                  // Param count
    buffer[3] = 1;                  // Param #1 length
    buffer[4] = pin;                // Param #1
    buffer[5] = 1;                  // Param #2 length
    buffer[6] = value;              // Param #2
    buffer[7] = NINA_CMD_END;       // Packet end
    
    // Select CS
    nina_wait_for_ready();
    gpio_put(NINA_PIN_SPI_CS, false);

    // Send Command
    uint32_t bytes_sent = spi_write_blocking(NINA_SPI, buffer, sizeof(buffer));
    
    // Deselect CS
    gpio_put(NINA_PIN_SPI_CS, true);
}


static void nina_wait_for_ready(void) {

    uint64_t last = time_us_64();
    while (time_us_64() - last > 6000) {
        if (gpio_get(NINA_PIN_READY)) {
            break;
        }
    }
}
