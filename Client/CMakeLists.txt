cmake_minimum_required(VERSION 3.13)
project(Client C)

set(CMAKE_C_STANDARD 99)

link_directories(/usr/lib/)

add_executable(Client main.c source/crud_functions.c source/connection.c source/readline_settings.c source/coords_msg.c)
target_link_libraries(Client readline jansson)