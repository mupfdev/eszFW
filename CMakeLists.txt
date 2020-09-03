cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

if(MSVC_IDE)
    set(SDL2_VERSION "2.0.12")
    set(SDL2_DEVEL_PKG SDL2-devel-${SDL2_VERSION}-VC.zip)

    include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

    ExternalProject_Add(SDL2_devel
        URL https://www.libsdl.org/release/${SDL2_DEVEL_PKG}
        URL_HASH SHA1=6839b6ec345ef754a6585ab24f04e125e88c3392
        DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/
        DOWNLOAD_NO_PROGRESS true
        TLS_VERIFY true
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2-${SDL2_VERSION}/
        BUILD_COMMAND cmake -E echo "Skipping build step."

        INSTALL_COMMAND cmake -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2-${SDL2_VERSION}/lib/x64/SDL2.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/demo

        PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeLists_SDL2_devel.txt"
        ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2-${SDL2_VERSION}/CMakeLists.txt)
endif(MSVC_IDE)

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
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    SYSTEM ${CWALK_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
    SYSTEM ${PICOLOG_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIRS}
    SYSTEM ${STB_INCLUDE_DIR})

set(eszFW_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_compat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_hash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_init.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_render.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_render.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/esz_utils.h)

set(demo_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/src/main.c)

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
    ${PICOLOG_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/demo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/demo)

add_executable(
    demo
    ${demo_sources})

if(WIN32)
    set_target_properties(
        demo
        PROPERTIES
        ADDITIONAL_CLEAN_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/SDL2.dll)
endif(WIN32)

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
    ${LUA_INCLUDE_DIR}/onelua.c)

option(ENABLE_DIAGNOSTICS "Enable all diagnostics"           OFF)
option(USE_LIBTMX         "Use libTMX instead of cute_tiled" OFF)

target_link_libraries(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    cwalk
    picolog)

target_link_libraries(
    demo
    ${SDL2_LIBRARIES}
    ${PROJECT_NAME})

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
        /W4)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND ENABLE_DIAGNOSTICS)
    message("Enabling all diagnostics")
    set(COMPILE_OPTIONS
        -Weverything)
    add_compile_options(-Weverything)
endif()
