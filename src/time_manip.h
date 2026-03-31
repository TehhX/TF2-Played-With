#ifndef TIME_MANIP_H
#define TIME_MANIP_H

/*
    time_manip.h
    ------------
    Contains time manipulation functionality

    Glossary:
        * UE-Seconds: Seconds since UNIX epoch
        * UE-Days: Days since UNIX epoch
*/

// Convert UE-seconds to UE-days
#define time_manip_ues2ued(UES) ((UES) / SECONDS_PER_DAY + 1)

// Convert UE-days to UE-seconds
#define time_manip_ued2ues(UED) ((UED) * SECONDS_PER_DAY)

// Internal use
#define _time_manip_print_tm(TM) printf("%4d-%02d-%02d", TM->tm_year + 1900, TM->tm_mon + 1, TM->tm_mday)

// Print date of UED
#define time_manip_print_ued(UED) { const struct tm *const _tm = localtime(&(time_t){ time_manip_ued2ues(UED) }); _time_manip_print_tm(_tm); }

// Print date of UES
#define time_manip_print_ues(UES) { const struct tm *const _tm = localtime(&(time_t){ UES }); _time_manip_print_tm(_tm); }

#endif // TIME_MANIP_H
