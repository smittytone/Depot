cmake_minimum_required(VERSION 3.30)

# Uncomment the following line to use the STEMMA connector for I2C.
#set(USE_STEMMA 1)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_7_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    LED_BUILD=1
    HW_MODEL="ADA-FEATHER-2040")

# Set the device's I2C pins
# These are for the board pins A0 (GPIO 26, SDA) and A1 (GPIO 27, SCL)
# You can set these to other I2C pins,
# but make sure you select pins on I2C1.
# The STEMMA connector's I2C pins are GPIO 2 (SDA) and 3 (SCL).
if (USE_STEMMA)
    add_compile_definitions(
        DEFAULT_SDA_PIN=2
        DEFAULT_SCL_PIN=3
        DEFAULT_I2C_BUS=1)
    unset(USE_STEMMA)
else()
    add_compile_definitions(
        DEFAULT_SDA_PIN=26
        DEFAULT_SCL_PIN=27
        DEFAULT_I2C_BUS=1)
endif()

# Include app source code file(s)
add_executable(${FW_7_NAME}
    ${FW_7_SRC_DIRECTORY}/main.c
    ${FW_7_SRC_DIRECTORY}/pico_led.c
    ${FW_7_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_7_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_7_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_7_NAME})
