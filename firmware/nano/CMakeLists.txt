cmake_minimum_required(VERSION 3.18)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_5_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    NANO_BUILD=1
    HW_MODEL="ARDUINO-NANO")

# Set the device's I2C pins
# These are for the board pins 1 (GPIO 0) and 2 (GPIO 1)
add_compile_definitions(
    DEFAULT_SDA_PIN=0
    DEFAULT_SCL_PIN=1
    DEFAULT_I2C_BUS=0)

# Include app source code file(s)
add_executable(${FW_5_NAME}
    ${FW_5_SRC_DIRECTORY}/main.c
    ${FW_5_SRC_DIRECTORY}/nano_led.c
    ${FW_5_SRC_DIRECTORY}/pins.c
    ${COMMON_CODE_DIRECTORY}/serial.c
    ${COMMON_CODE_DIRECTORY}/led.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${COMMON_CODE_DIRECTORY}/i2c.c
    ${COMMON_CODE_DIRECTORY}/onewire.c
)

# Compile debug sources
target_sources(${FW_5_NAME} PRIVATE "$<$<CONFIG:Debug>:${COMMON_CODE_DIRECTORY}/debug.c>")

# Set the stack size
target_compile_definitions(${FW_5_NAME} PRIVATE
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_5_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${FW_5_NAME} 1)
pico_enable_stdio_uart(${FW_5_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${FW_5_NAME})
