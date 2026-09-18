cmake_minimum_required(VERSION 3.16)
project(dash_abr_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(dash_abr_client
    main.c
    src/request_handler.c
    src/abr.c
)

target_include_directories(dash_abr_client PRIVATE include)
target_link_libraries(dash_abr_client PRIVATE CURL::libcurl)
target_compile_options(dash_abr_client PRIVATE -Wall -Wextra -Wpedantic)
