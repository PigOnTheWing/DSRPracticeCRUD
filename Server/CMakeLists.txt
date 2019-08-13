cmake_minimum_required(VERSION 3.13)
project(Server C)

set(CMAKE_C_STANDARD 99)

add_executable(server main.c source/conn_thread.c source/crud_functions.c source/json_parsing.c source/db_operations.c)

target_link_libraries(server jansson pthread messages)