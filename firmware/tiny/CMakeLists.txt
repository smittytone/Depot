cmake_minimum_required(VERSION 3.30)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_3_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    TINY_BUILD=1
    HW_MODEL="PIMORONI-TINY")

# Set the device's I2C pins
add_compile_definitions(
    DEFAULT_SDA_PIN=6
    DEFAULT_SCL_PIN=7
    DEFAULT_I2C_BUS=1)

# Include app source code file(s)
add_executable(${FW_3_NAME}
    ${FW_3_SRC_DIRECTORY}/main.c
    ${FW_3_SRC_DIRECTORY}/tiny_led.c
    ${FW_3_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_3_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_3_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pwm)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_3_NAME})
