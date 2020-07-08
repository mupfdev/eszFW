cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(LibXml2    REQUIRED)
find_package(SDL2       REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(
    SYSTEM external/cwalk/include
    SYSTEM external/tmx/src
    SYSTEM ${LIBXML2_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIRS}
    SYSTEM ${SDL2_IMAGE_INCLUDE_DIRS}
    SYSTEM ${SDL2_MIXER_INCLUDE_DIRS}
    src
)

add_subdirectory(external/tmx)
set_property(TARGET tmx PROPERTY POSITION_INDEPENDENT_CODE ON)

set(eszFW_SRCS
  external/cwalk/src/cwalk.c
  src/esz.c
  )

add_library(eszFW STATIC ${eszFW_SRCS})

target_link_libraries(
    eszFW
    tmx
    ${LIBXML2_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARY}
)

if (UNIX)
    target_link_libraries(eszFW m)
endif (UNIX)
