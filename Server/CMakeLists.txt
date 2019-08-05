cmake_minimum_required(VERSION 3.13)
project(Server C)

set(CMAKE_C_STANDARD 99)

add_executable(server main.c source/conn_thread.c source/crud_functions.c)

target_link_libraries(server jansson pthread messages)