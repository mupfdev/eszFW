project(RainbowJoe C)

FILE(GLOB JOE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_executable(rainbowjoe ${JOE_SRCS})
target_link_libraries(rainbowjoe eszFW)

add_subdirectory(..)
