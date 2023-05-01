cmake_minimum_required(VERSION 3.18)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_0_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    LED_BUILD=1
    HW_MODEL="PI-PICO"
)

# Set the device's I2C pins
# These are for the board pins 4 (GPIO 2) and 5 (GPIO 3)
# You can set these to other I2C pins but make sure
# you select pins on I2C1
add_compile_definitions(
    DEFAULT_SDA_PIN=2
    DEFAULT_SCL_PIN=3
    DEFAULT_I2C_BUS=1
)

# Include app source code file(s)
add_executable(${FW_0_NAME}
    ${FW_0_SRC_DIRECTORY}/main.c
    ${FW_0_SRC_DIRECTORY}/pico_led.c
    ${FW_0_SRC_DIRECTORY}/pins.c
    ${COMMON_CODE_DIRECTORY}/serial.c
    ${COMMON_CODE_DIRECTORY}/led.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${COMMON_CODE_DIRECTORY}/i2c.c
    ${COMMON_CODE_DIRECTORY}/onewire.c
    ${COMMON_CODE_DIRECTORY}/button.c
)

# Compile debug sources
target_sources(${FW_0_NAME} PRIVATE "$<$<CONFIG:Debug>:${COMMON_CODE_DIRECTORY}/debug.c>")

# Set the stack size
target_compile_definitions(${FW_0_NAME} PRIVATE
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192
)

# Link to built libraries
target_link_libraries(${FW_0_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${FW_0_NAME} 1)
pico_enable_stdio_uart(${FW_0_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${FW_0_NAME})
