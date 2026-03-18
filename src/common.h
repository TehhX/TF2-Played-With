#ifndef COMMON_H
#define COMMON_H

#ifdef TF2_PLAYED_WITH_DEBUG

    #include "stdio.h"
    #include "stdlib.h"

    // ON  Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) CONTENT

    // ON  Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION) if (CONDITION) abort()

#else

    // OFF Gets replaced with CONTENT
    #define TF2_PLAYED_WITH_DEBUG_INSERT(CONTENT) ;

    // OFF Aborts the program if CONDITION
    #define TF2_PLAYED_WITH_DEBUG_ABORT_IF(CONDITION)

#endif

#endif // COMMON_H
