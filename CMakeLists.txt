cmake_minimum_required(VERSION 3.28)
project(TF2-Played-With LANGUAGES C VERSION 0.0.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(${PROJECT_NAME}
    src/main.c
)
