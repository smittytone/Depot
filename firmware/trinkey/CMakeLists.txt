cmake_minimum_required(VERSION 3.30)

# Uncomment the following line to use the STEMMA
set(USE_STEMMA 1)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_4_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    NEO_BUILD=1
    HW_MODEL="ADA-TRINKEY-2040")

# Set the device's I2C pins
# These are for the board pins 5 (GPIO 2) and 6 (GPIO 3)
# You can set these to other I2C pins but make sure
# you select pins on I2C1
if (USE_STEMMA)
    add_compile_definitions(
        DEFAULT_SDA_PIN=16
        DEFAULT_SCL_PIN=17
        DEFAULT_I2C_BUS=0)
    unset(USE_STEMMA)
endif()

# NeoPixel
add_compile_definitions(
    PIN_NEO_DATA=27
    PIN_NEO_PWR=0
    NEO_COLOUR=0x101800)    # Yellow

# Include app source code file(s)
add_executable(${FW_4_NAME}
    ${FW_4_SRC_DIRECTORY}/main.c
    ${FW_4_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_4_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_4_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pio)

# NOTE This driver is shared with the QTPy and ProMicro boards,
#      so it lives under the common code directory
target_sources(${FW_4_NAME} PRIVATE ${COMMON_CODE_DIRECTORY}/ws2812.c)
pico_generate_pio_header(${FW_4_NAME} ${COMMON_CODE_DIRECTORY}/ws2812.pio)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_4_NAME})
