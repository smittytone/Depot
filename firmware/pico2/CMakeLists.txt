cmake_minimum_required(VERSION 3.30)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_6_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    LED_BUILD=1
    HW_MODEL="PI-PICO-2") # 24 characters max.

# Set the device's I2C pins
# These are for the board pins 4 (GPIO 2) and 5 (GPIO 3)
# You can set these to other I2C pins but make sure
# you select pins on I2C1
add_compile_definitions(
    DEFAULT_SDA_PIN=2
    DEFAULT_SCL_PIN=3
    DEFAULT_I2C_BUS=1)

# Include app source code file(s)
add_executable(${FW_6_NAME}
    ${FW_6_SRC_DIRECTORY}/main.c
    ${FW_6_SRC_DIRECTORY}/pico_led.c
    ${FW_6_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_6_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_6_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_6_NAME})
