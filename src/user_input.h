#ifndef USER_INPUT_H
#define USER_INPUT_H

/*
    user_input.h
    ------------
    Contains functionality for user input via the CLI
*/

#include "common.h"

#include "stdbool.h"

typedef void (*sigint_action_t)(const char *prompt);

/*
    @brief Get a line of user input

        @param input A pointer to a character pointer to receive the line once completed, to be free'd by user
        @param prompt The prompt to print before reading, or NULL for no prompt
        @param sigint_action If not NULL, it's called when SIGINT gets caught, else use default SIGINTs behavior

        @returns `*input` if successful, NULL if failed

        @warning Make sure `*line` is either NULL or malloc'd
        @warning Check return value is not NULL
*/
extern char *user_input_getline(char **input, const char *prompt, sigint_action_t sigint_action) TF2PW_ATTR_NONNULL(1);

/*
    @brief Get user confirmation for an operation. Will keep asking until single character Y/N (case insensitive) is entered

        @param prompt The prompt to display beforehand
        @param catch_sigints Pass

        @returns True if Y/y, false if N/n

        @note Uses `user_input_getline(...)` internally
*/
extern bool user_input_confirm(const char *prompt, sigint_action_t sigint_action);

#endif // USER_INPUT_H
