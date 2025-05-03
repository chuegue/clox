cmake_minimum_required(VERSION 3.13)

project(codecrafters-interpreter)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 23)

# Add debug flags for GCC/Clang
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")  # Debug symbols + disable optimizations
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3") # Keep release optimizations
endif()

add_executable(interpreter ${SOURCE_FILES})

target_link_libraries(interpreter m)