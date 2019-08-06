cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(CRUD VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDART 99)

link_directories(/usr/lib/)
include_directories(lib/messages/)

add_subdirectory(lib/messages/ messages/)
add_subdirectory(Server/ Server/)
add_subdirectory(Client/ Client/)