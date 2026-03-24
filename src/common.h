#ifndef COMMON_H
#define COMMON_H

/*
    common.h
    --------
    Contains definitions to be used in *all* files contained within this project
*/

// Define by setting config to DEBUG in CMake, or by defining the following value in your own compiling efforts
#ifdef TF2_PLAYED_WITH_DEBUG
    #include "stdio.h"
    #include "stdlib.h"

    // ON  Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) CONTENT

    // ON  Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION) if (CONDITION) abort()

    // ON  When debugging, printf's MSG. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_LOGF printf

    // ON  When debugging, abort. Else, exit EXIT_FAILURE. For when you want your debugger to stop here, but in regular operations for it to just exit with an error code
    #define TF2_PLAYED_WITH_DEBUG_ABEX abort

    // ON  When debugging, abort. Else, no-op
    #define TF2_PLAYED_WITH_DEBUG_ABORT abort

    // DEB When debugging, insert DEB. When not, insert REL
    #define TF2_PLAYED_WITH_DEBUG_CHOOSE(DEB, REL) DEB
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
#endif

#endif // COMMON_H
