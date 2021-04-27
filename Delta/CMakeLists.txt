cmake_minimum_required(VERSION 3.13)
project(TPDelta)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()


include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/CLASS ${PROJECT_SOURCE_DIR}/CRC ${PROJECT_SOURCE_DIR}/UTHASH)
add_subdirectory(CLASS)
add_subdirectory(CRC)

add_library(coder ENCODER.c DECODER.c)

add_executable(TPDelta main.c)
target_link_libraries(coder PRIVATE class crc)
target_link_libraries(TPDelta PRIVATE class crc coder)