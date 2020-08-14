cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(SDL2 REQUIRED)
if(USE_LIBTMX)
    find_package(LibXml2 REQUIRED)
endif(USE_LIBTMX)

set(CUTE_INCLUDE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/external/cute_headers)
set(CWALK_INCLUDE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/external/cwalk/include)
set(LIBTMX_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/external/tmx/src)
set(LUA_INCLUDE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/external/lua)
set(PICOLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/picolog)
set(STB_INCLUDE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)

include_directories(
    SYSTEM ${CWALK_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
    SYSTEM ${PICOLOG_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIRS}
    SYSTEM ${STB_INCLUDE_DIR})

set(eszFW_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_types.h)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${eszFW_sources})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CUTE_INCLUDE_DIR}
    ${LIBTMX_INCLUDE_DIR}
    ${PICOLOG_INCLUDE_DIR}
    )

add_library(
    cwalk
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cwalk/src/cwalk.c)

add_library(
    picolog
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/picolog/picolog.c)

add_library(
    lua
    STATIC
    ${LUA_INCLUDE_DIR}/onelua.c
    )

option(ENABLE_DIAGNOSTICS "Enable all diagnostics"           OFF)
option(USE_LIBTMX         "Use libTMX instead of cute_tiled" OFF)

target_link_libraries(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    cwalk
    picolog)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(USE_LIBTMX)
    add_definitions(-DUSE_LIBTMX)
    add_subdirectory(external/tmx)
    set_property(TARGET tmx PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(
        ${PROJECT_NAME}
        ${LIBXML2_LIBRARIES}
        tmx)
endif(USE_LIBTMX)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif(UNIX)

if (CMAKE_C_COMPILER_ID     STREQUAL "Clang")
    set(COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(COMPILE_OPTIONS
        -W4)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND ENABLE_DIAGNOSTICS)
    message("Enabling all diagnostics")
    set(COMPILE_OPTIONS
        -Weverything)
    add_compile_options(-Weverything)
endif()
