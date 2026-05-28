#include "array_manip.h"

#include "common.h"

#include "stdint.h"

// Evaluates to PTR, but all related pointer arithmetic will use intervals of one byte instead of the size of their original dereferenced types
#define PTR_ARITH(PTR) ((uint8_t *) (PTR))

struct array_manip_find_return array_manip_find(const void *const needle, void *const haystack, const size_t size, const size_t count, array_manip_find_compare_t data_compare)
{
    if (count == 0)
    {
        return (struct array_manip_find_return){ .index = 0, .status = array_manip_find_status_start };
    }
    else if (data_compare(needle, PTR_ARITH(haystack) + size * (count - 1)) > 0)
    {
        return (struct array_manip_find_return){ .index = count, .status = array_manip_find_status_prospective };
    }
    else if (data_compare(needle, haystack) < 0)
    {
        return (struct array_manip_find_return){ .status = array_manip_find_status_start };
    }

    size_t low = 0, high = count - 1, mid;
    while (low <= high)
    {
        mid = low + (high - low) / 2;

        const int difference = data_compare(PTR_ARITH(haystack) + size * mid, needle);

        if (difference == 0)
        {
            return (struct array_manip_find_return){ .index = size * mid, .status = array_manip_find_status_found };
        }
        else if (difference < 0)
        {
            low = mid + 1;
        }
        else if (difference > 0)
        {
            high = mid - 1;
        }
    }

    return (struct array_manip_find_return){ .index = size * mid, .status = array_manip_find_status_prospective };
}
