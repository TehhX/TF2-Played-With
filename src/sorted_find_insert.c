#include "sorted_find_insert.h"

#include "string.h"

void sorted_find_insert(const void *const value, void **const array, size_t *const array_len, const size_t element_size, sorted_find_insert_action_t on_find, sorted_find_insert_action_t on_insert, void *const params, const __compar_fn_t compare)
{
    if (*array_len == 0)
    {
        memcpy(*array = malloc(element_size), value, element_size);
        on_insert(*array, params);
        ++*array_len;
        return;
    }
    else if (compare(*array, value) > 0)
    {
        *array = realloc(*array, element_size * ++*array_len);
        memmove(*array + element_size, *array, element_size * (*array_len - 1));
        on_insert(*array, params);
        return;
    }
    else if (compare(*array + element_size * (*array_len - 1), value) < 0)
    {
        *array = realloc(*array, element_size * ++*array_len);
        on_insert(*array + element_size * (*array_len - 1), params);
        return;
    }

    size_t low = 0, high = *array_len - 1;
    while (low < high)
    {
        const size_t mid = low + (high - low) / 2;
        const int compare_result = compare(*array + element_size * mid, value);

        if (compare_result == 0)
        {
            on_find(*array + element_size * mid, params);
            return;
        }
        else if (compare_result < 0)
        {
            low = mid + 1;
        }
        else // Implicitly compare_result > 0
        {
            high = mid - 1;
        }
    }

    if (compare(*array + element_size * low, value) == 0)
    {
        on_find(*array + element_size * low, params);
        return;
    }

    if (compare(value, *array + element_size * low) > 0)
    {
        ++low;
    }

    *array = realloc(*array, element_size * ++*array_len);
    memmove(*array + element_size * (low + 1), *array + element_size * low, element_size * (*array_len - low - 1));
    on_insert(*array + element_size * low, params);
}
