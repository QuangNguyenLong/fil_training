cmake_minimum_required(VERSION 3.0)

project(sample_udp C)

# include_directories(${PROJECT_SOURCE_DIR}/include)

# file(GLOB SHARED_SOURCE
#     ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(udp_sender ${SHARED_SOURCE} sender/main.c)
add_executable(udp_receiver ${SHARED_SOURCE} receiver/main.c)
