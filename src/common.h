#ifndef COMMON_H
#define COMMON_H

/*
    common.h
    --------
    Contains definitions to be used in *all* files contained within this project
*/

// Check that OS is supported
#if !defined(_WIN32) && !defined(__linux__)
    #error "Unknown OS."
#endif

// Compiler specific definitions
#ifdef __GNUC__
    #define TF2PW_ATTR_ALWINL __attribute__((always_inline))
    #define TF2PW_ATTR_NONNULL(INDICES) __attribute__((nonnull (INDICES)))
#elif defined(_MSC_VER)
    #define TF2PW_ATTR_ALWINL __forceinline
    #define TF2PW_ATTR_NONNULL(INDICES)
#else
    #error "Unknown compiler."
#endif

// Define by setting config to DEBUG in CMake, or by defining the following value in your own compiling efforts
#ifdef TF2_PLAYED_WITH_DEBUG
    #include "stdio.h"
    #include "stdlib.h"

    // ON  Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) CONTENT

    // ON  Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION) if (CONDITION) abort()

    // TODO: Make a wrapper that always prints LOG and uses ANSI_LOG, a lot of dupe code
    // ON  When debugging, printf's MSG. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_LOGF printf

    // ON  When debugging, abort. Else, exit EXIT_FAILURE. For when you want your debugger to stop here, but in regular operations for it to just exit with an error code
    #define TF2_PLAYED_WITH_DEBUG_ABEX abort

    // ON  When debugging, abort. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_ABORT abort

    // DEB When debugging, insert DEB. When not, insert REL
    #define TF2_PLAYED_WITH_DEBUG_CHOOSE(DEB, REL) DEB

    // OFF A function acting as a glorified macro
    #define HYPER_MACRO
#else
    // OFF Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT)

    // OFF Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION)

    // OFF When debugging, printf's MSG. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_LOGF

    // OFF When debugging, abort. Else, exit EXIT_FAILURE. For when you want your debugger to stop here, but in regular operations for it to just exit with an error code
    #define TF2_PLAYED_WITH_DEBUG_ABEX() exit(EXIT_FAILURE)

    // OFF When debugging, abort. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_ABORT()

    // REL When debugging, insert DEB. When not, insert REL
    #define TF2_PLAYED_WITH_DEBUG_CHOOSE(DEB, REL) REL

    // ON  A function acting as a glorified macro
    #define HYPER_MACRO static inline TF2PW_ATTR_ALWINL
#endif

// Literal tab. A tab is just 4 spaces to a guy like me
#define LTAB "    "

// ANSI Color Escape Codes (Disable by passing below def)
#ifdef NO_ANSI_COLORING
    #define ANSI_RED
    #define ANSI_GREEN
    #define ANSI_YELLOW
    #define ANSI_BLUE
    #define ANSI_MAGENTA
    #define ANSI_CYAN
    #define ANSI_RESET

    // OFF Sets the color of output STR to ANSI color COLOR
    #define SET_COLOR(STR, COLOR)
#else
    #define ANSI_RED    "\033[31m"
    #define ANSI_GREEN  "\033[32m"
    #define ANSI_YELLOW "\033[33m"
    #define ANSI_BLUE   "\033[34m"
    #define ANSI_CYAN   "\033[36m"
    #define ANSI_RESET  "\033[00m"

    // ON  Sets the color of output STR to ANSI color COLOR
    #define SET_COLOR(STR, COLOR) fprintf(STR, COLOR)
#endif // NO_ANSI_COLORING

// ANSI aliases
#define ANSI_LOG ANSI_CYAN
#define RESET_STDERR_COL() SET_COLOR(stderr, ANSI_RESET)

// Check that characters C1 and C2 match, regardless of capitalization
#define ccasecmp(C1, C2) (((C1) | 0x20) == ((C2) | 0x20))

// Proper realloc without repeated code
#define prealloc(PTR, SIZE, LEN) PTR = realloc(PTR, (SIZE) * (LEN))

// Copy string `S` to heap
#define string_deep_copy(S) strcpy(malloc(strlen(S) + 1), S)

// Print string without newline
#define putsnnl(S) printf("%s", S); fflush(stdout)

#endif // COMMON_H
