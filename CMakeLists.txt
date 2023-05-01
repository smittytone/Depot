cmake_minimum_required(VERSION 3.18)

# Set project data
set(PROJECT_NAME "Depot Bus Host Firmware")
set(FW_BUILD_NUMBER "46")
set(FW_VERSION_NUMBER "1.2.3")

# Set app name(s) and version(s)
# NOTE Indices changed from 1.2.2
set(FW_0_NAME "firmware_pico")
set(FW_0_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/pico")

set(FW_1_NAME "firmware_qtpy_rp2040")
set(FW_1_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/qtpy")

set(FW_2_NAME "firmware_promicro")
set(FW_2_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/promicro")

set(FW_3_NAME "firmware_tiny2040")
set(FW_3_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/tiny")

set(FW_4_NAME "firmware_trinkey2040")
set(FW_4_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/trinkey")

# FROM 1.2.2
set(FW_5_NAME "firmware_arduino_nano")
set(FW_5_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/nano")

# FROM 1.2.2
# Create a list of Depot-supported PICO_BOARD values
set(BOARD_LIST  "pico"
                "adafruit_qtpy_rp2040"
                "sparkfun_promicro"
                "pimoroni_tiny2040"
                "adafruit_trinkey_qt2040"
                "arduino_nano_rp2040_connect")
set(NAME_LIST   "Pico"
                "Adafruit QTPy RP2040"
                "SparkFun ProMicro"
                "Pimoroni Tiny 2040"
                "Adafruit Trinkey QT2040"
                "Arduino Nano RP2040 Connect")

# Move common source code to a separate directory
set(COMMON_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/firmware/common")

# Set app-side debugging on debug builds
# Now uses CMake generator expression to extract config type
add_compile_definitions($<$<CONFIG:Debug>:DO_UART_DEBUG=1>)

# Show the bus host's pulse
add_compile_definitions(SHOW_HEARTBEAT=1)

# Set env variable 'PICO_SDK_PATH' to the local Pico SDK
# Comment out the set() if you have a global copy of the
# SDK set and $PICO_SDK_PATH defined in your $PATH
# set(ENV{PICO_SDK_PATH} "${CMAKE_SOURCE_DIR}/pico-sdk")

# Set flags and directory variables
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DTSDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTSDEBUG")

# Include the Pico SDK
include(pico_sdk_import.cmake)

# FROM 1.1.3
# Recommend usage of SDK 1.5.0+
if(PICO_SDK_VERSION_MINOR LESS "5")
message(ERROR " WE STRONGLY RECOMMEND YOU BUILD THE FIRMWARE WITH PICO SDK 1.5.0 OR ABOVE")
endif()

# Name the project
project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        VERSION ${FW_VERSION_NUMBER}
        DESCRIPTION "RP2040-based Multi-bus adaptor"
        HOMEPAGE_URL https://smittytone.net/depot
)

# FROM 1.2.2
# Check for a board type set using the Pico SDK's `PICO_BOARD` env var
set(BOARD_INDEX -1)
if(DEFINED ENV{PICO_BOARD})
    set(PICO_BOARD $ENV{PICO_BOARD})
    list(FIND BOARD_LIST ${PICO_BOARD} BOARD_INDEX)
    if(BOARD_INDEX EQUAL -1)
        message("[DEPOT] Environmemnt variable PICO_BOARD set to unsupported board '${PICO_BOARD}'.")
    else()
        message("[DEPOT] Environmemnt variable PICO_BOARD set to supported board '${PICO_BOARD}'.")
    endif()
else()
    message("[DEPOT] Environmemnt variable PICO_BOARD undefined.")
endif()

# Initialise the Pico SDK
pico_sdk_init()

# FROM 1.2.2
# Build for a specific board, or for all
if(BOARD_INDEX EQUAL -1)
    # Include all the apps' source code
    message("[DEPOT] Configuring builds for all supported boards.")
    add_subdirectory(${FW_0_SRC_DIRECTORY}) # Pico
    add_subdirectory(${FW_1_SRC_DIRECTORY}) # QTPy
    add_subdirectory(${FW_2_SRC_DIRECTORY}) # ProMicro
    add_subdirectory(${FW_3_SRC_DIRECTORY}) # Tiny
    add_subdirectory(${FW_4_SRC_DIRECTORY}) # Trinkey
    add_subdirectory(${FW_5_SRC_DIRECTORY}) # Arduino Nano
else()
    list(GET NAME_LIST ${BOARD_INDEX} BOARD)
    message("[DEPOT] Configuring build for ${BOARD}.")
    set(BLD_DIR "FW_${BOARD_INDEX}_SRC_DIRECTORY")
    add_subdirectory("${${BLD_DIR}}")
endif()
