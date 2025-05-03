cmake_minimum_required(VERSION 3.13)
project(codecrafters-interpreter C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Explicit source file listing (better than GLOB)
file(GLOB_RECURSE SOURCE_FILES 
    src/*.c
)

# Debug/Release configurations
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
endif()

add_executable(interpreter ${SOURCE_FILES})

# Include directories for header files
target_include_directories(interpreter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(interpreter m)

message(STATUS "Source files: ${SOURCE_FILES}")