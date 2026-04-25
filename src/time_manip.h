#ifndef TIME_MANIP_H
#define TIME_MANIP_H

/*
    time_manip.h
    ------------
    Contains time manipulation functionality

    Glossary:
        * UE-Seconds (UES): Seconds since UNIX epoch
        * UE-Days    (UED): Days since UNIX epoch
*/

#include "time.h"

// Seconds in a day
#define SECONDS_PER_DAY (24 * 60 * 60)

// Convert UE-seconds to UE-days
#define time_manip_ues2ued(UES) ((uint16_t) (((UES) / SECONDS_PER_DAY) + 1))

// Convert UE-days to UE-seconds
#define time_manip_ued2ues(UED) ((int32_t) ((UED) * SECONDS_PER_DAY))

// The current time in UES
#define time_manip_current_ues() time(NULL)

// The current time in UED
#define time_manip_current_ued() (time_manip_ues2ued(time_manip_current_ues()))

// Internally used by `time_manip_print_ued` and `time_manip_print_ued`
#define _time_manip_print_tm printf("%4d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday)

// Print date of UED
#define time_manip_print_ued(UED) { const struct tm *const tm = localtime(&(time_t){ time_manip_ued2ues(UED) }); _time_manip_print_tm; }

// Print date of UES
#define time_manip_print_ues(UES) { const struct tm *const tm = localtime(&(time_t){ UES }); _time_manip_print_tm; }

#endif // TIME_MANIP_H
