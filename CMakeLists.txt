cmake_minimum_required(VERSION 3.28)
project(tf2pw LANGUAGES C VERSION 0.0.0 DESCRIPTION "A program to keep track of who you've played with in Valve's Team Fortress 2." HOMEPAGE_URL "https://github.com/TehhX/TF2-Played-With")

if(NOT LINUX)
    message(STATUS "This software is not tested on platforms other than linux. Continue at your own risk by commenting this out. Take special care with argp.")
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

# In case includes aren't in accessible spots, user may put them here
target_include_directories(tf2pw PRIVATE "include/")
target_link_directories   (tf2pw PRIVATE "lib/")

target_link_libraries(tf2pw PRIVATE cider)

# Enable debugging when variant is "Debug"
if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    # Compiler/linker options
    target_compile_definitions(tf2pw PRIVATE "TF2_PLAYED_WITH_DEBUG=1")

    if (CMAKE_C_COMPILER_ID STREQUAL GNU)
        target_compile_options(tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter" "-Wno-switch")
        target_link_options   (tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined")
    elseif (CMAKE_C_COMPILER_ID STREQUAL MSVC)
        target_compile_options(tf2pw PRIVATE "/fsanitize=address" "/fsanitize=undefined")
        target_link_options   (tf2pw PRIVATE "/fsanitize=address" "/fsanitize=undefined")
    endif ()
endif ()

# Use -DTF2_PLAYED_WITH_NO_ANSI_COLORING when configuring to disable ANSI color escapes
if (DEFINED TF2_PLAYED_WITH_NO_ANSI_COLORING)
    target_compile_definitions(tf2pw PRIVATE TF2_PLAYED_WITH_NO_ANSI_COLORING=1)
endif ()
