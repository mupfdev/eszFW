cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(SDL2 REQUIRED)
find_package(LibXml2)

set(CUTE_INCLUDE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/external/cute_headers)
set(CWALK_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/external/cwalk/include)
set(LIBTMX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tmx/src)
set(LUA_INCLUDE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/external/lua)
set(STB_INCLUDE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)

include_directories(
    SYSTEM ${CWALK_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIRS}
    SYSTEM ${STB_INCLUDE_DIR})

set(eszFW_sources
    src/esz.c)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${eszFW_sources})

target_include_directories(
   ${PROJECT_NAME}
   PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
   PUBLIC ${CUTE_INCLUDE_DIR}
   PUBLIC ${LIBTMX_INCLUDE_DIR})

add_library(
    cwalk
    STATIC
    external/cwalk/src/cwalk.c)

add_library(
    lua
    STATIC
    ${LUA_INCLUDE_DIR}/lapi.c
    ${LUA_INCLUDE_DIR}/lauxlib.c
    ${LUA_INCLUDE_DIR}/lbaselib.c
    ${LUA_INCLUDE_DIR}/lcode.c
    ${LUA_INCLUDE_DIR}/lcorolib.c
    ${LUA_INCLUDE_DIR}/lctype.c
    ${LUA_INCLUDE_DIR}/ldblib.c
    ${LUA_INCLUDE_DIR}/ldebug.c
    ${LUA_INCLUDE_DIR}/ldo.c
    ${LUA_INCLUDE_DIR}/ldump.c
    ${LUA_INCLUDE_DIR}/lfunc.c
    ${LUA_INCLUDE_DIR}/lgc.c
    ${LUA_INCLUDE_DIR}/linit.c
    ${LUA_INCLUDE_DIR}/liolib.c
    ${LUA_INCLUDE_DIR}/llex.c
    ${LUA_INCLUDE_DIR}/lmathlib.c
    ${LUA_INCLUDE_DIR}/lmem.c
    ${LUA_INCLUDE_DIR}/loadlib.c
    ${LUA_INCLUDE_DIR}/lobject.c
    ${LUA_INCLUDE_DIR}/lopcodes.c
    ${LUA_INCLUDE_DIR}/loslib.c
    ${LUA_INCLUDE_DIR}/lparser.c
    ${LUA_INCLUDE_DIR}/lstate.c
    ${LUA_INCLUDE_DIR}/lstring.c
    ${LUA_INCLUDE_DIR}/lstrlib.c
    ${LUA_INCLUDE_DIR}/ltable.c
    ${LUA_INCLUDE_DIR}/ltablib.c
    ${LUA_INCLUDE_DIR}/ltests.c
    ${LUA_INCLUDE_DIR}/ltm.c
    ${LUA_INCLUDE_DIR}/lua.c
    ${LUA_INCLUDE_DIR}/lundump.c
    ${LUA_INCLUDE_DIR}/lutf8lib.c
    ${LUA_INCLUDE_DIR}/lvm.c
    ${LUA_INCLUDE_DIR}/lzio.c
    ${LUA_INCLUDE_DIR}/onelua.c
)

option(USE_CUTE_TILED "Use cute_tiled instead of libTMX" OFF)

target_link_libraries(
  ${PROJECT_NAME}
  ${SDL2_LIBRARIES}
  cwalk)

if(USE_CUTE_TILED)
  add_definitions(-DUSE_CUTE_TILED)
else()
  add_definitions(-DUSE_LIBTMX)
  add_subdirectory(external/tmx)
  set_property(TARGET tmx PROPERTY POSITION_INDEPENDENT_CODE ON)
  target_link_libraries(
    ${PROJECT_NAME}
    ${LIBXML2_LIBRARIES}
    tmx)
endif(USE_CUTE_TILED)

if(UNIX)
    target_link_libraries(eszFW m)
endif(UNIX)

if(USE_CLANG)
    set (CC "clang")
    target_compile_options(eszFW PRIVATE -Weverything -Wno-unused-function)
endif(USE_CLANG)
