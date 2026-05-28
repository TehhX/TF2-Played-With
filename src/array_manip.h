#ifndef ARRAY_MANIP_H
#define ARRAY_MANIP_H

/*
    array_manip.h
    -------------

    Some utilities and functions for manipulating arrays of all types
*/

#include "stdlib.h"

// @brief Returns the index of `ELEMENT` within `ARRRAY`. Element size is specified by `ELEMENT_SIZE`
#define ARRAY_MANIP_INDEX_OF_SIZED(ARRAY, ELEMENT, ELEMENT_SIZE) (((size_t) (ELEMENT) - (size_t) (ARRAY)) / (ELEMENT_SIZE))

// @brief Returns the index of `ELEMENT` within `ARRRAY`. Element size is determined via `ARRAY`
#define ARRAY_MANIP_INDEX_OF(ARRAY, ELEMENT) ARRAY_MANIP_INDEX_OF_SIZED(ARRAY, ELEMENT, sizeof(*(ARRAY)))

// IMMED_TODO: Might only need found, and not found. If not found, prospective/start. Else, found. Index will portray the remaining information
// @brief An enum describing different possible statuses of `array_manip_find(...)`
enum array_manip_find_status
{
    array_manip_find_status_found,       // @brief Requested value was found at returned index
    array_manip_find_status_prospective, // @brief Requested value was not found, but would be placed at returned index if it were
    array_manip_find_status_start        // @brief Requested value was not found, but would be placed at the beginning
};

// @brief A struct describing the findings of `array_manip_find(...)`
struct array_manip_find_return
{
    size_t index; // @brief The index of an element. It's relation to `needle` is described by its sister datum `status`
    enum array_manip_find_status status; // @brief The status of `array_manip_find(...)`'s results, to be used in conjunction with its sister datum `index`
};

/*
    @brief The type for a callback function used to compare elements

        @param needle This will contain a pointer to the needle provided by the user which is currently being checked against `haystack_element`
        @param haystack_element This will contain a pointer to an element of `haystack` currently being checked against `needle`

        @returns If the user defined value of `needle` is greater than that of `haystack element`, it will return a value greater than 0. If they are of equal value, it will return 0. If `needle` is of lesser value, it will return a value lesser than 0.
*/
typedef int (*array_manip_find_compare_t)(const void *needle, const void *haystack_element);

/*
    @brief Finds the index of an element needle in haystack

        @param needle The element to find
        @param haystack The array within which to search
        @param size The size of any given element
        @param count How many elements are in the array `haystack`
        @param data_compare The function to compare elements with. See comments on `array_manip_find_compare_t` for further clarification

        @returns A struct describing its findings. See comments on `array_manip_find_return` for further clarification
*/
extern struct array_manip_find_return array_manip_find(const void *const needle, void *const haystack, const size_t size, const size_t count, array_manip_find_compare_t data_compare);

#endif // ARRAY_MANIP_H
