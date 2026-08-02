cmake_minimum_required(VERSION 3.30)

# Uncomment the following line to use the STEMMA connector for I2C.
#set(USE_STEMMA 1)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_2_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    NEO_BUILD=1
    HW_MODEL="SF-PRO-MICRO")

# Set the device's I2C pins
# These are for the board pins 5 (GPIO 2, SDA) and 6 (GPIO 3, SCL)
# You can set these to other I2C pins,
# but make sure you select pins on I2C1.
# The STEMMA connector's I2C pins are GPIO 16 (SDA) and 17 (SCL).
if (USE_STEMMA)
    add_compile_definitions(
        DEFAULT_SDA_PIN=16
        DEFAULT_SCL_PIN=17
        DEFAULT_I2C_BUS=0)
    unset(USE_STEMMA)
else()
    add_compile_definitions(
        DEFAULT_SDA_PIN=2
        DEFAULT_SCL_PIN=3
        DEFAULT_I2C_BUS=1)
endif()

# NeoPixel
add_compile_definitions(
    PIN_NEO_DATA=25
    PIN_NEO_PWR=7
    NEO_COLOUR=0x000010)    # Blue

# Include app source code file(s)
add_executable(${FW_2_NAME}
    ${FW_2_SRC_DIRECTORY}/main.c
    ${FW_2_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_2_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_2_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pio)

# NOTE This driver is shared with the QTPy and Trinkey boards,
#      so it lives under the common code directory
target_sources(${FW_2_NAME} PRIVATE ${COMMON_CODE_DIRECTORY}/ws2812.c)
pico_generate_pio_header(${FW_2_NAME} ${COMMON_CODE_DIRECTORY}/ws2812.pio)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_2_NAME})
