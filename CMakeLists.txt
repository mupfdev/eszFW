cmake_minimum_required(VERSION 3.10)
project(eszFW C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(LibXml2 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(
        SYSTEM external/tmx/src
        SYSTEM ${LIBXML2_INCLUDE_DIR}
        SYSTEM ${SDL2_INCLUDE_DIRS}
        SYSTEM ${SDL2_IMAGE_INCLUDE_DIRS}
        SYSTEM ${SDL2_MIXER_INCLUDE_DIRS}
        src
)

add_subdirectory(external/tmx)
set_property(TARGET tmx PROPERTY POSITION_INDEPENDENT_CODE ON)

FILE(GLOB eszFW_SRCS src/*.c)
add_library(eszFW SHARED ${eszFW_SRCS})

target_link_libraries(
        eszFW
        tmx
        ${LIBXML2_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARY}
)

target_compile_options(eszFW PRIVATE -pedantic-errors -Wall -Werror -Wextra)
target_compile_options(tmx PRIVATE -pedantic-errors -Wall -Werror -Wextra)

set_property(TARGET eszFW PROPERTY INTERPROCEDURAL_OPTIMIZATION True)

if (UNIX)
    target_link_libraries(eszFW m)
endif (UNIX)
