cmake_minimum_required(VERSION 3.28)
project(TF2-Played-With LANGUAGES C VERSION 0.1.0)

if(NOT LINUX)
    message(FATAL_ERROR "This software is not tested on platforms other than linux. Continue at your own risk by commenting this out. Take special care with argp.")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

set(SRC
    history
    main
)

# Each item "{}" in SRC becomes "src/{}.c".
list(TRANSFORM SRC PREPEND "src/")
list(TRANSFORM SRC  APPEND ".c")

add_executable(TF2-Played-With ${SRC})

target_link_libraries(TF2-Played-With PRIVATE cider)

# Enable debugging when variant is "Debug"
if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    target_compile_definitions(TF2-Played-With PRIVATE TF2_PLAYED_WITH_DEBUG)
    target_compile_options    (TF2-Played-With PRIVATE "-fsanitize=address" "-fsanitize=undefined")
    target_link_options       (TF2-Played-With PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif ()
