cmake_minimum_required(VERSION 3.28)
project(TF2-Played-With LANGUAGES C VERSION 0.1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(TF2-Played-With
    src/main.c
)

if(NOT LINUX)
    message(FATAL_ERROR "This software is not tested on platforms other than linux. Continue at your own risk by commenting this out. Take special care with argp.")
endif()
