cmake_minimum_required(VERSION 3.10)
project(explore-mms C)

add_executable(explore-mms src/explore-mms.c)

find_library(IEC61850_LIB NAMES libiec61850.a)

if (NOT IEC61850_LIB OR NOT HAL_LIB)
    message(FATAL_ERROR "libiec61850.a not found!")
endif()

target_link_libraries(explore-mms PRIVATE ${IEC61850_LIB})
target_link_options(explore-mms PRIVATE -static)
