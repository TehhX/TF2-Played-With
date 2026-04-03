#ifndef STEAMID_MANIP_H
#define STEAMID_MANIP_H

/*
    steamid_manip.h
    ---------------
    Contains utilities for manipulating different forms of STEAMID and player names
*/

#include "stdint.h"

// Flags to specify possible STEAMID types
enum Esteamid_type
{
    Esteamid_type_unknown, // Not known, test all
    Esteamid_type_sid3,    // STEAMID3
    Esteamid_type_sid3e,   // STEAMID3 Excerpt
    Esteamid_type_sid64,   // STEAMID64
    Esteamid_type_name,    // Steam username
};

// Potential return values for errors
#define SIDM_ERR_MISC     ((uint32_t) UINT32_MAX - 0) // Miscellaneous error
#define SIDM_ERR_RNGE     ((uint32_t) UINT32_MAX - 1) // Range error
#define SIDM_ERR_NAME     ((uint32_t) UINT32_MAX - 2) // Attempted to parse name
#define SIDM_ERR_NONE_MAX ((uint32_t) UINT32_MAX - 3) // Maximum value of no error (no error if (return_value <= SIDM_ERR_NONE_MAX))

// The difference between a STEAMID3 and STEAMID64
#define SIDM_ID_DIFFERENCE ((uint64_t) 0x110000100000000)

// Converts a STEAMID64 to a STEAMID3E
#define sidm_sid64_to_sid3e(SID64) ((uint32_t) ((SID64) - SIDM_ID_DIFFERENCE))

// Converts a STEAMID3E to a STEAMID64
#define sidm_sid3e_to_sid64(SID3E) ((uint32_t) ((SID3E) + SIDM_ID_DIFFERENCE))

/*
    Parses a STEAMID3 excerpt from string input, useful for user input. Returns SIDM_ERR if a failure occurred

        @param string_input: A string representation of any possible form of STEAMID. Should be NULL terminated, except for STEAMID3 which only requires an `]` at the end
        @param expected_type: STEAMID type contained in `string_input`, or `Esteamid_type_unknown` for unknown. Don't pass name `Esteamid_type_name`
*/
extern uint32_t sidm_parse_sid3e(const char *string_input, enum Esteamid_type possible_types);

#endif // STEAMID_MANIP_H
