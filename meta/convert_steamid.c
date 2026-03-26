// Demonstrates conversion of STEAMID's for later implementation in source code

#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "inttypes.h"
#include "stdlib.h"

// The difference between a STEAMID3 and STEAMID64
#define ID_DIFFERENCE ((uint64_t) 76561197960265728)

uint64_t convert_364(uint32_t in_3)
{
    return ((uint64_t) in_3) + ID_DIFFERENCE;
}

uint32_t convert_643(uint64_t in_64)
{
    return (uint32_t) (in_64 - ID_DIFFERENCE);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: %s [364|643] [ID]\n    - 364:  STEAMID3 -> STEAMID64\n    - 643: STEAMID64 ->  STEAMID3\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "364"))
    {
        printf("STEAMID64: %" PRIu64 "\n", convert_364(atoll(argv[2])));
        return 0;
    }
    else if (!strcmp(argv[1], "643"))
    {
        printf("STEAMID3: %" PRIu32 "\n", convert_643(atoll(argv[2])));
        return 0;
    }
    else
    {
        printf("Bad option arg.\n");
        return 1;
    }
}
