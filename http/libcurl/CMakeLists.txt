cmake_minimum_required(VERSION 3.0)

project(sample_curl C)

add_executable(curl_get main.c)

target_link_libraries(curl_get curl)
