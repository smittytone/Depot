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


Nano_LED_colour colour;

static uint8_t SLAVESELECT = 10; // ss
static uint8_t SLAVEREADY  = 7;  // handshake pin
static uint8_t SLAVERESET  = 5;  // reset pin


/**
 * @brief Initialise the Nina module and its SPI link
 */
void nano_led_init() {

    // Initialise SPI link to Nina/ESP32

    // SET Nina/ESP32 GPIO pins to OUT
    gpio_init(NINA_PIN_RSTN);
    gpio_set_dir(NINA_PIN_RSTN, GPIO_OUT);
    gpio_put(NINA_PIN_RSTN, false);

    gpio_init(NINA_PIN_GPIO0);
    gpio_set_dir(NINA_PIN_GPIO0, GPIO_OUT);
    gpio_put(NINA_PIN_GPIO0, false);

    gpio_init(NINA_PIN_READY);
    gpio_set_dir(NINA_PIN_READY, GPIO_IN);

    gpio_init(NINA_PIN_SPI_CS);
    gpio_set_dir(NINA_PIN_SPI_CS, GPIO_OUT);
    gpio_put(NINA_PIN_SPI_CS, true);

    gpio_put(NINA_PIN_GPIO0, true);
    gpio_put(NINA_PIN_RSTN, true);
    sleep_ms(10);
    gpio_put(NINA_PIN_RSTN, false);
    sleep_ms(750);
    gpio_put(NINA_PIN_GPIO0, false);

    gpio_put(NINA_PIN_GPIO0, false);
    gpio_set_dir(NINA_PIN_GPIO0, GPIO_IN);

    // Init SPI1
    spi_init(NINA_SPI, 8000000);
    spi_set_format(NINA_SPI, 8, 0, 0, SPI_MSB_FIRST);

    gpio_set_function(NINA_PIN_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(NINA_PIN_SPI_RX, GPIO_FUNC_SPI);
    gpio_set_function(NINA_PIN_SPI_SCK, GPIO_FUNC_SPI);

    nina_pin_mode(NINA_LED_R, GPIO_OUT);
    nina_pin_mode(NINA_LED_G, GPIO_OUT);
    nina_pin_mode(NINA_LED_B, GPIO_OUT);

    nano_led_off();
}


/**
 * @brief Turn the LED off.
 */
void nano_led_off() {

    // SET Nina/ESP32 GPIO pins to HIGH
    nina_analog_write(NINA_LED_R, 255);
    nina_analog_write(NINA_LED_G, 255);
    nina_analog_write(NINA_LED_B, 255);
}


/**
 * @brief Turn the LED on.
 */
void nano_led_on() {

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


void nina_pin_mode(uint32_t pin, uint32_t mode) {

    nina_send_cmd(NINA_CMD_SET_PIN_MODE, pin, mode);
}


void nina_send_cmd(uint32_t cmd, uint32_t pin, uint32_t value) {

    WAIT_FOR_SLAVE_SELECT();

    // Send Command
    spi_send_cmd(cmd, 2);
    spi_send_param((uint8_t*)&pin, 1, false);
    spi_send_param((uint8_t*)&value, 1, true);

    // pad to multiple of 4
    spi_transfer('D');  // 'D' for Dummy
    DELAY_TRANSFER();

    spi_slave_deselect();
    //Wait the reply elaboration
    spi_wait_for_slave_ready();
    spi_slave_select();

    // Wait for reply
    uint8_t _data = 0;
    uint8_t _dataLen = 0;

    if (!SpiDrv::waitResponseCmd(cmd, PARAM_NUMS_1, &_data, &_dataLen)) {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }

    spi_slave_deselect();
}


void nina_analog_write(uint32_t pin, uint32_t value) {

    nina_send_cmd(NINA_CMD_ANALOG_WRITE, pin, value);

}


void spi_wait_for_slave_ready(void) {

    unsigned long const start = millis();
	while (digitalRead(SLAVEREADY) != LOW) {
        // NOP
    }
}


void spi_send_cmd(uint8_t cmd, uint8_t num_params){

    // Send SPI START CMD
    spi_transfer(START_CMD);

    // Send SPI C + cmd
    spi_transfer(cmd & ~(REPLY_FLAG));

    // Send SPI numParam
    spi_transfer(num_params);

    // If numParam == 0 send END CMD
    if (num_params == 0) spiTransfer(END_CMD);
}


void spi_send_param(uint8_t* param, uint8_t param_len, bool is_last_param) {

    // Send SPI paramLen
    spi_transfer(param_len);

    // Send SPI param data
    for (uint32_t i = 0 ; i < param_len ; ++i) {
        spi_transfer(param[i]);
    }

    // if lastParam==1 Send SPI END CMD
    if (is_last_param) spi_transfer(END_CMD);
}


inline uint32_t spi_transfer(volatile uint8_t data) {

    return spi_write_blocking(NINA_SPI, &data, 1);
}


void spi_slave_deselect(void) {

    gpio_put(NINA_PIN_SPI_CS, true);
}


void spi_slave_select(void) {

    gpio_put(NINA_PIN_SPI_CS, false);

    // wait for up to 5 ms for the NINA to indicate it is not ready for transfer
    // the timeout is only needed for the case when the shield or module is not present
    for (unsigned long start = millis(); (digitalRead(SLAVEREADY) != HIGH) && (millis() - start) < 5;);
}


unsigned int spi_wait_response_cmd(uint8_t cmd, uint8_t numParam, uint8_t* param, uint8_t* param_len)
{
    char _data = 0;
    int ii = 0;

    IF_CHECK_START_CMD(_data) {
        CHECK_DATA(cmd | REPLY_FLAG, _data){};
        CHECK_DATA(numParam, _data)
        {
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
