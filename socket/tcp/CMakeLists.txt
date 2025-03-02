cmake_minimum_required(VERSION 3.0)

project(sample_tcp C)

# include_directories(${PROJECT_SOURCE_DIR}/include)

# file(GLOB SHARED_SOURCE
#     ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(tcp_client ${SHARED_SOURCE} client/main.c)
add_executable(tcp_server ${SHARED_SOURCE} server/main.c)


