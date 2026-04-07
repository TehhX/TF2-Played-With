#include "user_input.h"

#include "common.h"

#include "stdlib.h"
#include "stdio.h"

#ifdef __linux__
    #include "readline/readline.h"
#endif

void user_input_getline(char **const input, const char *const prompt)
{
    RETRY_INPUT:;

    free(*input);
#ifdef __linux__
    *input = readline(prompt);

    // User entered nothing
    if (!*input || !*input[0])
    {
        goto RETRY_INPUT;
    }
#elif defined(_WIN32) || defined(_WIN64)
    if (prompt)
    {
        printf(prompt);
    }

    char *buff = NULL;
    int next;
    size_t len = 0;

    // BUFF_TODO
    while ((next = fgetc(stdin)) != '\n')
    {
        prealloc(buff, sizeof(char), ++len);
        buff[len - 1] = (char) next;
    }

    prealloc(buff, sizeof(char), len + 1);
    buff[len] = '\0';

    if (!*input[0])
    {
        free(buff);
        goto RETRY_INPUT;
    }
    else
    {
        *input = buff;
    }
#else
    #error "Unknown OS."
#endif
}

bool user_input_confirm(const char *const prompt)
{
    char *input = NULL;
    bool retval;

    RETRY_CONFIRMATION:;
    user_input_getline(&input, prompt);

    if ((input[0] | CAPITAL_BIT) == 'y')
    {
        if (input[1] == '\0')
        {
            retval = true;
        }
        else
        {
            goto RETRY_CONFIRMATION;
        }
    }
    else if ((input[0] | CAPITAL_BIT) == 'n')
    {
        if (input[1] == '\0')
        {
            retval = false;
        }
        else
        {
            goto RETRY_CONFIRMATION;
        }
    }
    else
    {
        goto RETRY_CONFIRMATION;
    }

    free(input);
    return retval;
}
