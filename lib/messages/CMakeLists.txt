cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)

project(messages)

add_library(messages STATIC ../include/messages.h messages.c)