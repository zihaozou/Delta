project(TPDelta)

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()


include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/CLASS ${PROJECT_SOURCE_DIR}/CRC ${PROJECT_SOURCE_DIR}/UTHASH ${PROJECT_SOURCE_DIR}/MD5)
add_subdirectory(CLASS)
add_subdirectory(CRC)
add_subdirectory(MD5)
add_library(coder ENCODER.c DECODER.c)

add_executable(TPDelta main.c)
target_link_libraries(coder PRIVATE class crc md5)
target_link_libraries(TPDelta PRIVATE class crc coder md5)
