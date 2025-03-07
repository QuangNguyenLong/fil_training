cmake_minimum_required(VERSION 3.0)

project(sample_c C)

# skip this line if you are on section II
option(SOME_FEATURE "Enable something" OFF)

# want to include header without using absolute path?
# add this  
include_directories(${PROJECT_SOURCE_DIR}/include)

# specify where the source files for header are at
file(GLOB SHARED_SOURCE
    ${PROJECT_SOURCE_DIR}/src/*.c)

# tell gcc to compile your program
add_executable(my_program ${SHARED_SOURCE} my_program/main.c)

if(SOME_FEATURE)
    # define SOME_FEATURE in the program my_program
    target_compile_definitions(my_program PRIVATE SOME_FEATURE)
endif()