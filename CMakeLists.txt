cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(LibXml2 REQUIRED)
find_package(SDL2    REQUIRED)

include_directories(
    SYSTEM external/cute_headers
    SYSTEM external/cwalk/include
    SYSTEM external/stb
    SYSTEM external/tmx/src
    SYSTEM ${LIBXML2_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIRS}
    src
)

set(eszFW_SRCS
    external/cwalk/src/cwalk.c
    src/esz.c
    )

add_library(
    ${PROJECT_NAME}
    STATIC
    ${eszFW_SRCS})

option(USE_TMXLIB "Use tmxlib instead of cute_tiled" ON)

if(USE_TMXLIB)
  add_definitions(-DUSE_TMXLIB)
  add_subdirectory(external/tmx)
  set_property(TARGET tmx PROPERTY POSITION_INDEPENDENT_CODE ON)
  target_link_libraries(
    ${PROJECT_NAME}
    tmx
    ${LIBXML2_LIBRARIES}
    ${SDL2_LIBRARIES})
else()
  target_link_libraries(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES})
endif()

if (UNIX)
    target_link_libraries(eszFW m)
endif (UNIX)

if(USE_CLANG)
    set (CC "clang")
    target_compile_options(eszFW PRIVATE -Weverything -Wno-unused-function)
endif(USE_CLANG)
