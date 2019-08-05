cmake_minimum_required(VERSION 3.13)
project(Client C)

set(CMAKE_C_STANDARD 99)

add_executable(client main.c source/json_parsing.c source/connection.c source/readline_settings.c)

target_link_libraries(client readline jansson messages)