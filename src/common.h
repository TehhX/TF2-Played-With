#ifndef COMMON_H
#define COMMON_H

#ifdef TF2_PLAYED_WITH_DEBUG

    #include "stdio.h"
    #include "stdlib.h"

    // ON  Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) CONTENT

    // ON  Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION) if (CONDITION) abort()

    // ON  Printf's MSG
    #define TF2_PLAYED_WITH_DEBUG_LOGF printf

    // ON  When debugging, abort. Else, exit EXIT_FAILURE. For when you want your debugger to stop here, but in regular operations for it to just exit with an error code
    #define TF2_PLAYED_WITH_DEBUG_ABEX abort()

#else

    // OFF Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) ;

    // OFF Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION)

    // OFF Printf's MSG
    #define TF2_PLAYED_WITH_DEBUG_LOGF ;

    // OFF When debugging, abort. Else, exit EXIT_FAILURE. For when you want your debugger to stop here, but in regular operations for it to just exit with an error code
    #define TF2_PLAYED_WITH_DEBUG_ABEX exit(EXIT_FAILURE)

#endif

#endif // COMMON_H
