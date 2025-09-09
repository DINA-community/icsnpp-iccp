cmake_minimum_required(VERSION 3.10)
project(explore-mms C)

add_executable(explore-mms src/explore-mms.c)

find_path(IEC61850_INCLUDE_DIR
    NAMES iec61850_common.h
    PATHS /usr/include /usr/local/include /usr/local/include/libiec61850
)

find_library(IEC61850_LIB NAMES libiec61850.a)
if (NOT IEC61850_LIB)
    message(FATAL_ERROR "libiec61850.a not found!")
endif()

find_library(HAL_LIB NAMES libhal.a)
if (NOT HAL_LIB)
    message(FATAL_ERROR "libhal.a not found!")
endif()

target_link_libraries(explore-mms PRIVATE ${IEC61850_LIB} ${HAL_LIB})
target_include_directories(explore-mms PRIVATE ${IEC61850_INCLUDE_DIR})
target_link_options(explore-mms PRIVATE -static)
