#ifndef USER_INPUT_H
#define USER_INPUT_H

/*
    user_input.h
    ------------
    Contains functionality for user input on the CLI
*/

#include "stdbool.h"

/*
    @brief Get a line of user input

        @param input The character pointer to return into
        @param prompt The prompt to print before reading, or NULL for none

        @returns A string containing the whole line of input. NULL terminated (not '\n')

        @warning `input` is free'd before assignment, make sure is either NULL or contains malloc data
        @warning Return value is malloc'd
*/
extern void user_input_getline(char **input, const char *prompt);

/*
    @brief Get user confirmation for an operation. Will keep asking until single character Y/N (case insensitive) is entered

        @param prompt The prompt to display beforehand

        @returns True if Y/y, false if N/n
*/
extern bool user_input_confirm(const char *prompt);

#endif // USER_INPUT_H
