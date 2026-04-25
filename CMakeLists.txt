cmake_minimum_required(VERSION 3.28)
project("TF2-Played-With" LANGUAGES C VERSION 0.0.0 DESCRIPTION "A program to keep track of who you've played with in Valve's Team Fortress 2." HOMEPAGE_URL "https://github.com/TehhX/TF2-Played-With")
file(WRITE "${CMAKE_SOURCE_DIR}/version.txt" "${PROJECT_VERSION_MAJOR}-${PROJECT_VERSION_MINOR}-${PROJECT_VERSION_PATCH}")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

set(SRC
    collection
    history
    interactive
    main
    steamid_manip
    user_input
)

# Each item "{}" in SRC becomes "src/{}.c".
list(TRANSFORM SRC PREPEND "src/")
list(TRANSFORM SRC  APPEND ".c")

add_executable(tf2pw ${SRC})

# In case includes aren't in accessible spots, user may put them here
target_include_directories(tf2pw PRIVATE "include/")
target_link_directories   (tf2pw PRIVATE "lib/")

target_link_libraries(tf2pw PRIVATE cider)

# winpthreads, see /README-technical.md
if (WIN32)
    target_link_libraries(tf2pw PRIVATE pthreadVC3)
elseif (LINUX)
    target_link_libraries(tf2pw PRIVATE readline)
endif ()

# Enable debugging when variant is "Debug"
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Compiler/linker options
    target_compile_definitions(tf2pw PRIVATE "TF2_PLAYED_WITH_DEBUG=1")

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-Wall" "-Wextra" "-Wno-unused-parameter")
        target_link_options   (tf2pw PRIVATE "-fsanitize=address" "-fsanitize=undefined")
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(MSVC_DISABLED_WARNINGS
            "4255" # Warns when using foo() {} instead of foo(void) {}
            "4100" # Warns when parameters aren't used
            "5045" # Spectre warnings, some security vulnerability from 2017
            "4820" # Notifies when struct padding occurs
            "4152" # Warns if object pointer is converted to function pointer or vice versa
        )

        list(TRANSFORM MSVC_DISABLED_WARNINGS PREPEND "/wd")

        target_compile_options(tf2pw PRIVATE "/fsanitize=address" "/W4" ${MSVC_DISABLED_WARNINGS})
    endif ()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Nothing to do
elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    # Nothing to do
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # Nothing to do
else ()
    message(FATAL_ERROR "Specify a build type, eg. -DCMAKE_BUILD_TYPE:STRING=Release.")
endif ()

# Use -DNO_ANSI_COLORING=1 when configuring to disable ANSI color escapes
if (NO_ANSI_COLORING EQUAL 1)
    target_compile_definitions(tf2pw PRIVATE NO_ANSI_COLORING=${NO_ANSI_COLORING})
endif ()

# Installation
install(TARGETS "tf2pw" DESTINATION "bin")
