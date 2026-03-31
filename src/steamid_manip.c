#include "steamid_manip.h"

#include "common.h"
#include "time_manip.h"

#include "stdlib.h"
#include "ctype.h"
#include "errno.h"

// Explicitly parse SID3E from type SID3 contained in `string_input`
HYPER_MACRO uint32_t parse_exp_sid3(const char *const restrict string_input)
{
    errno = 0;

    char *end;
    const uint32_t current_sid3e = strtol(string_input + sizeof("[U:1:") - 1, &end, 10);

    // Bad input misc
    if (*end != ']')
    {
        return SIDM_ERR_MISC;
    }
    // Bad input range
    else if (errno == ERANGE)
    {
        errno = 0;
        return SIDM_ERR_RNGE;
    }

    return current_sid3e;
}

// Explicitly parse SID3E from type SID64 contained in `string_input`
HYPER_MACRO uint32_t parse_exp_sid64(const char *const restrict string_input)
{
    errno = 0;

    char *end;
    const uint32_t current_sid64 = strtol(string_input, &end, 10);

    // Bad input misc
    if (*end != '\0')
    {
        return SIDM_ERR_MISC;
    }
    // Bad input range
    else if (errno == ERANGE)
    {
        errno = 0;
        return SIDM_ERR_RNGE;
    }

    return current_sid64;
}

// Explicitly parse SID3E from type SID3 contained in `string_input`
HYPER_MACRO uint32_t parse_exp_sid3e(const char *const restrict string_input)
{
    errno = 0;

    char *end;
    const uint32_t current_sid3 = strtol(string_input, &end, 10);

    // Bad input misc
    if (*end != '\0')
    {
        return SIDM_ERR_MISC;
    }
    // Bad input range
    else if (errno == ERANGE)
    {
        errno = 0;
        return SIDM_ERR_RNGE;
    }

    return current_sid3;
}

uint32_t sidm_parse_sid3e(const char *const restrict string_input, const enum Esteamid_type possible_types)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (possible_types == Esteamid_type_name)
        {
            fprintf(stderr, ANSI_RED "FATAL: Passed Esteamid_type_name to sidm_parse_sid3e(...).possible_types.\n" ANSI_RESET);
            abort();
        }
    )

    // TODO: Stress test unknown capabilities
    // Unknown type, parse and try all
    if (possible_types == Esteamid_type_unknown)
    {
        // STEAMID3
        if (string_input[0] == '[')
        {
            return parse_exp_sid3(string_input);
        }
        else
        {
            errno = 0;

            char *end;
            const uint64_t value = strtol(string_input, &end, 10);

            // Bad input misc
            if (*end != '\0')
            {
                return SIDM_ERR_MISC;
            }
            // Bad input range
            else if (errno == ERANGE)
            {
                errno = 0;
                return SIDM_ERR_RNGE;
            }
            // STEAMID64
            else if (value > UINT32_MAX)
            {
                return sidm_sid64_to_sid3e(value);
            }
            // STEAMID3E
            else
            {
                return value;
            }
        }
    }
    // Type is explicit
    else
    {
        switch (possible_types)
        {
            case Esteamid_type_sid3:  return parse_exp_sid3(string_input);
            case Esteamid_type_sid3e: return parse_exp_sid3e(string_input);
            case Esteamid_type_sid64: return parse_exp_sid64(string_input);
        }
    }

    // Shouldn't get here
    abort();
}
