cmake_minimum_required(VERSION 3.30)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_7_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    #LED_BUILD=1
    NEO_BUILD=1
    HW_MODEL="ADAFRUIT-FEATHER-2040") # 24 characters max.

# Set the device's I2C pins
# These are for the board pins SDA (GPIO 2) and SCL (GPIO 3, SCL)
# The STEMMA connector's I2C pins are also GPIO 2 (SDA) and 3 (SCL).
add_compile_definitions(
    DEFAULT_SDA_PIN=2
    DEFAULT_SCL_PIN=3
    DEFAULT_I2C_BUS=1)

# NeoPixel
add_compile_definitions(
    PIN_NEO_DATA=16
    PIN_NEO_PWR=4
    NEO_COLOUR=0xFF00FF) # Magenta?

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
    hardware_i2c
    hardware_pio)

# Compile WS2828 sources
# NOTE This driver is shared with the ProMicro and Trinkey boards,
#      so it lives under the common code directory
target_sources(${FW_7_NAME} PRIVATE ${COMMON_CODE_DIRECTORY}/ws2812.c)
pico_generate_pio_header(${FW_7_NAME} ${COMMON_CODE_DIRECTORY}/ws2812.pio)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_7_NAME})
