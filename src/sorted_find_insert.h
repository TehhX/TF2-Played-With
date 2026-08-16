#ifndef SORTED_FIND_INSERT_H
#define SORTED_FIND_INSERT_H

/*
    sorted_find_insert.h
    --------------------

    Contains functionality for inserting elements in ordered lists
*/

#include "stdlib.h"

/*
    @brief A callback type for when a value is found/inserted into an array via `sorted_find_insert(...)`

        @param array_element The element of the array that was found/inserted
        @param params Additional params passed through `sorted_find_insert(...)`
*/
typedef void (*const sorted_find_insert_action_t)(void *const array_element, void *const params);

/*
    @brief Will insert `*value` into sorted array `*array` if it is not present, or find it if it is. Then, will perform `on_insert` or `on_find` respectively

        @param array A pointer to an array of items, sorted in ascending order according to `compare`
        @param array_len A pointer to the length of the array `*array`
        @param element_size The size, in bytes, of each individual element in `*array`
        @param value The value to find/insert
        @param on_find A function to execute when finding `*value` in `*array`
        @param on_insert A function to execute when inserting `*value` in `*array`
        @param params Additional information to be passed along to relevant action functions. Pass NULL for no additional information
        @param compare A function to compare two elements in `*array`
*/
extern void sorted_find_insert(const void *const value, void **const array, size_t *const array_len, const size_t element_size, sorted_find_insert_action_t on_find, sorted_find_insert_action_t on_insert, void *const params, const __compar_fn_t compare);

#endif // SORTED_FIND_INSERT_H
