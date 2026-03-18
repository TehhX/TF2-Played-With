#include "stdint.h"
#include "string.h"

// 1 to check and refuse invalid inputs, 0 to just segfault on invalid inputs (when input is known to be good)
#define DEBUG_VALUE (int) 0

// Takes STEAMID3 as constant string, returns STEAMID64 equivalent as u64. Returns 0 if DEBUG_VALUE is set AND input is invalid
static uint64_t steamid3_to_64(const char *steamid3)
{
    // Check input. Not neccessary if input is known to be good. More for debug builds
    if (DEBUG_VALUE && (strncmp(steamid3, "[U:1:", sizeof("[U:1:") - 1) || steamid3[strlen(steamid3) - 1] != ']'))
    {
        return 0;
    }

    uint64_t return_value = 0;

    // 5 is the index of first 'X' in hypothetical STEAMID3 "[U:1:XXXXXXX]". Will end at ']'
    for (int i = 5; steamid3[i] != ']'; ++i)
    {
        return_value = return_value * 10 + steamid3[i] - '0';
    }

    // Magic number from /meta/steamdb_id_conversion.js
    return 76561197960265728 | return_value;
}

// Only required for printing in main()
#include "inttypes.h"
#include "stdio.h"

// Prints STEAMID3 -> STEAMID64 conversions for each provided argument, or INVALID ID3 if a bad STEAMID3 is input
int main(int argc, char **argv)
{
    --argc; ++argv;

    if (argc < 1)
    {
        fputs("Need > 1 args of type STEAMID3.\n", stderr);
        return 1;
    }

    printf
    (
        "        STEAMID3 |         STEAMID64\n"
        "-----------------+------------------\n"
    );

    for (int i = 0; i < argc; ++i)
    {
        const uint64_t steamid_64 = steamid3_to_64(argv[i]);

        printf("%16s | ", argv[i]);

        if (steamid_64)
        {
            printf("%" PRIu64 "\n", steamid_64);
        }
        else
        {
            puts("INVALID ID3");
        }
    }
}
