cmake_minimum_required(VERSION 3.30)

# Uncomment the following line to use the STEMMA
#set(USE_STEMMA 1)

# Make project data accessible to compiler
# IMPORTANT `HW_MODEL` must be no more than 24 characters
add_compile_definitions(
    FW_NAME="${FW_1_NAME}"
    FW_VERSION="${FW_VERSION_NUMBER}"
    BUILD_NUM=${FW_BUILD_NUMBER}
    NEO_BUILD=1
    HW_MODEL="ADA-QTPY-RP2040")

# Set the device's I2C pins
# These are for the board pins A2 (SCL) and A3 (SDA)
# NOT SDA and SCL. This is to ensure compatibility with
# the STEMMA port -- both A2,A3,STEMMA all used i2c1.
if (USE_STEMMA)
    add_compile_definitions(
        DEFAULT_SDA_PIN=26
        DEFAULT_SCL_PIN=27
        DEFAULT_I2C_BUS=1)
    unset(USE_STEMMA)
else()
    add_compile_definitions(
        DEFAULT_SDA_PIN=22
        DEFAULT_SCL_PIN=23
        DEFAULT_I2C_BUS=1)
endif()

# NeoPixel
add_compile_definitions(
    PIN_NEO_DATA=12
    PIN_NEO_PWR=11
    NEO_COLOUR=0x100010)    # Magenta

# Include app source code file(s)
add_executable(${FW_1_NAME}
    ${FW_1_SRC_DIRECTORY}/main.c
    ${FW_1_SRC_DIRECTORY}/pins.c)

# Set the stack size
# NOTE Kept per-board as this may vary by device in future
target_compile_definitions(${FW_1_NAME} PRIVATE
    PICO_CRT0_ALLOCATE_SPACERS=0
    PICO_HEAP_SIZE=8192
    PICO_STACK_SIZE=8192)

# Link to built libraries
target_link_libraries(${FW_1_NAME} LINK_PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_pio)

# Compile WS2828 sources
# NOTE This driver is shared with the ProMicro and Trinkey boards,
#      so it lives under the common code directory
target_sources(${FW_1_NAME} PRIVATE ${COMMON_CODE_DIRECTORY}/ws2812.c)
pico_generate_pio_header(${FW_1_NAME} ${COMMON_CODE_DIRECTORY}/ws2812.pio)

# Apply the common firmware build configuration (common sources,
# debug sources, STDIO and extra build products)
depot_configure_firmware(${FW_1_NAME})
