cmake_minimum_required(VERSION 3.18)

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
    ${FW_4_SRC_DIRECTORY}/pins.c
    ${COMMON_CODE_DIRECTORY}/serial.c
    ${COMMON_CODE_DIRECTORY}/led.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${COMMON_CODE_DIRECTORY}/i2c.c
    ${COMMON_CODE_DIRECTORY}/onewire.c)

# Compile debug sources
# Now uses CMake generator expression to extract config type
target_sources(${FW_4_NAME} PRIVATE "$<$<CONFIG:Debug>:${COMMON_CODE_DIRECTORY}/debug.c>")

# Set the stack size
target_compile_definitions(${FW_4_NAME} PRIVATE
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_4_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pio)

target_sources(${FW_4_NAME} PRIVATE ${FW_1_SRC_DIRECTORY}/ws2812.c)
pico_generate_pio_header(${FW_4_NAME} ${FW_1_SRC_DIRECTORY}/ws2812.pio)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${FW_4_NAME} 1)
pico_enable_stdio_uart(${FW_4_NAME} 0)

# Enable extra build products
pico_add_extra_outputs(${FW_4_NAME})
