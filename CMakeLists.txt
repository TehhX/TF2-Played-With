cmake_minimum_required(VERSION 3.28)
project(tf2pw LANGUAGES C VERSION 0.2.0)

if(NOT LINUX)
    message(FATAL_ERROR "This software is not tested on platforms other than linux. Continue at your own risk by commenting this out. Take special care with argp.")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

set(SRC
    collection
    history
    interactive
    main
    steamid_manip
)

# Each item "{}" in SRC becomes "src/{}.c".
list(TRANSFORM SRC PREPEND "src/")
list(TRANSFORM SRC  APPEND ".c")

add_executable(tf2pw ${SRC})

target_link_libraries(tf2pw PRIVATE cider)

# Enable debugging when variant is "Debug"
if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    target_compile_definitions(tf2pw PRIVATE TF2_PLAYED_WITH_DEBUG=1)
    target_compile_options    (tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined")
    target_link_options       (tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif ()

# Use -DTF2_PLAYED_WITH_NO_ANSI_COLORING when configuring to disable ANSI color escapes
if (DEFINED TF2_PLAYED_WITH_NO_ANSI_COLORING)
    target_compile_definitions(tf2pw PRIVATE TF2_PLAYED_WITH_NO_ANSI_COLORING=1)
endif ()
