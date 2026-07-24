#ifndef SORTED_FIND_INSERT_H
#define SORTED_FIND_INSERT_H

/*
    sorted_find_insert.h
    ---------------

    Contains functionality for inserting elements in ordered lists
*/

#include "stdlib.h"

typedef void (*const sorted_find_insert_action_t)(void *const array_element, const size_t element_size, const void *value);

extern void sorted_find_insert(void **const array, size_t *const array_len, const size_t element_size, const void *const value, sorted_find_insert_action_t on_find, sorted_find_insert_action_t on_insert, const __compar_fn_t compare);

#endif // SORTED_FIND_INSERT_H
