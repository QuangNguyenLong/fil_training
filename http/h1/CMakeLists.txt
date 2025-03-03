cmake_minimum_required(VERSION 3.0)

project(sample_http C)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SHARED_SOURCE
    ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(http_client ${SHARED_SOURCE} client/main.c)

