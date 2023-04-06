cmake_minimum_required(VERSION 3.18)

# Set project data
set(PROJECT_NAME "Depot Clients for Linux")
set(VERSION_NUMBER "1.2.2")
set(BUILD_NUMBER "43")

# Set app name(s) and version(s)
set(CLI2C_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/cli2c")
set(MATRIX_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/matrix")
set(SEGMENT_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/segment")
set(CLIWIRE_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/cliwire")
set(COMMON_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/common")
set(I2C_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/i2c")
set(ONEWIRE_CODE_DIRECTORY "${CMAKE_SOURCE_DIR}/../client/onewire")

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
    ${CLIWIRE_CODE_DIRECTORY})

# Name the project
project(${PROJECT_NAME}
        LANGUAGES C
        VERSION ${VERSION_NUMBER}
        DESCRIPTION "Command line multi-bus access"
        HOMEPAGE_URL https://smittytone.net/depot)

# Set the build version
add_compile_definitions(APP_VERSION="${VERSION_NUMBER}")

# Include app source code file(s)
add_executable(cli2c
    ${CLI2C_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c)

add_executable(matrix
    ${MATRIX_CODE_DIRECTORY}/main.c
    ${MATRIX_CODE_DIRECTORY}/ht16k33-matrix.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c)

add_executable(segment
    ${SEGMENT_CODE_DIRECTORY}/main.c
    ${SEGMENT_CODE_DIRECTORY}/ht16k33-segment.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${I2C_CODE_DIRECTORY}/i2cdriver.c)

add_executable(cliwire
    ${CLIWIRE_CODE_DIRECTORY}/main.c
    ${COMMON_CODE_DIRECTORY}/serialdriver.c
    ${COMMON_CODE_DIRECTORY}/utils.c
    ${COMMON_CODE_DIRECTORY}/gpio.c
    ${ONEWIRE_CODE_DIRECTORY}/owdriver.c)
