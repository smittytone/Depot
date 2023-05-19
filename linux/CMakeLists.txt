cmake_minimum_required(VERSION 3.20)

# Set project data
set(PROJECT_NAME "Depot Clients for Linux")
set(APP_VERSION_NUMBER "1.3.0")
set(APP_BUILD_NUMBER "46")

configure_file(app_version.in app_version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Set app name(s) and version(s)
set(CLI2C_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/cli2c")
set(MATRIX_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/matrix")
set(SEGMENT_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/segment")
set(CLIWIRE_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/cliwire")
set(COMMON_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/common")
set(I2C_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/i2c")
set(ONEWIRE_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/onewire")
# FROM 1.3.0
set(DS18B20_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/ds18b20")
set(BUTTONS_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/buttons")

# Set flags and directory variables
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DTSDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTSDEBUG")

add_compile_definitions(BUILD_FOR_LINUX=1)
#add_compile_definitions(DEBUG=1)

include_directories(
    ${COMMON_CODE_DIRECTORY}
    ${I2C_CODE_DIRECTORY}
    ${ONEWIRE_CODE_DIRECTORY}
    ${CLI2C_CODE_DIRECTORY}
    ${MATRIX_CODE_DIRECTORY}
    ${SEGMENT_CODE_DIRECTORY}
    ${CLIWIRE_CODE_DIRECTORY}
    ${DS18B20_CODE_DIRECTORY}
    ${BUTTONS_CODE_DIRECTORY}
)

# Name the project
project(${PROJECT_NAME}
        LANGUAGES C
        VERSION ${APP_VERSION_NUMBER}
        DESCRIPTION "Command line multi-bus access"
        HOMEPAGE_URL https://smittytone.net/depot
)

# Set the build version
#add_compile_definitions(APP_VERSION="${APP_VERSION_NUMBER}")
#add_compile_definitions(BUILD_NUM="${APP_BUILD_NUMBER}")

# Include app source code file(s)
add_executable(cli2c
    ${CLI2C_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c
)

add_executable(matrix
    ${MATRIX_CODE_DIRECTORY}/main.c
    ${MATRIX_CODE_DIRECTORY}/ht16k33-matrix.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c
)

add_executable(segment
    ${SEGMENT_CODE_DIRECTORY}/main.c
    ${SEGMENT_CODE_DIRECTORY}/ht16k33-segment.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c
)

add_executable(cliwire
    ${CLIWIRE_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${ONEWIRE_CODE_DIRECTORY}/owdriver.c
)

# FROM 1.3.0
add_executable(ds18b20
    ${DS18B20_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${ONEWIRE_CODE_DIRECTORY}/owdriver.c
)

# FROM 1.3.0
add_executable(buttons
    ${BUTTONS_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
)

# FROM 1.3.0 -- Auto increment build number
add_custom_command(OUTPUT INCREMENT_BUILD DEPENDS cli2c
    COMMAND python3 ../../scripts/increment.py
)

add_custom_target(do_increment ALL DEPENDS INCREMENT_BUILD)
