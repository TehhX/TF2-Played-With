#include "user_input.h"

#include "common.h"

#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "errno.h"

static bool caught_sigint = false;

#ifdef __linux__
    #include "signal.h"

    void sigint_handler(const int sig_code)
    {
        caught_sigint = true;
    }
#elif defined(_WIN32)
    #include "windows.h"

    BOOL WINAPI sigint_handler(const DWORD sig_code)
    {
        caught_sigint = true;

        return TRUE;
    }
#endif

static int cleanup(pthread_t *const thread, pthread_mutex_t *const mutex, char *input, const bool sigint_status)
{
    int retval = 0;

    if (thread)
    {
        if ((errno = pthread_cancel(*thread))
        // Windows complains without this
        #ifdef _WIN32
            && errno != ESRCH
        #endif
        )
        {
            perror("Cleanup: Couldn't cancel user input thread");
            retval = 1;
        }
    }

    if (mutex)
    {
        if ((errno = pthread_mutex_unlock(mutex)))
        {
            perror("Cleanup: Couldn't unlock mutex");
            retval = 1;
        }

        if ((errno = pthread_mutex_destroy(mutex)))
        {
            perror("Cleanup: Couldn't destroy mutex");
            retval = 1;
        }
    }

    // No-op if NULL anyway, no need to check first
    free(input);

    if (sigint_status)
    {
        if (
        #ifdef __linux__
            signal(SIGINT, NULL) == SIG_ERR
        #elif defined(_WIN32)
            !SetConsoleCtrlHandler(NULL, FALSE)
        #endif
        ){
            perror("Cleanup: Couldn't restore SIGINT handling");
            retval = 1;
        }
    }

    return retval;
}

struct routine_user_input_params
{
    char *input;
    size_t input_len;
    pthread_mutex_t input_lock;
};

static void *routine_user_input(struct routine_user_input_params *const params)
{
    pthread_mutex_lock(&params->input_lock);

    params->input_len = 0;

    for (bool cont = true; cont; )
    {
        // BUFF_TODO
        const int next = fgetc(stdin);
        switch (next)
        {
            break; case '\n':
            {
                params->input = realloc(params->input, sizeof(char) * ++params->input_len);
                params->input[params->input_len - 1] = '\0';

                cont = false;
            }
            break; case EOF:
            {
                params->input_len = 0;
            }
            break; default:
            {
                params->input = realloc(params->input, sizeof(char) * (params->input_len + 1));
                params->input[params->input_len++] = (char) next;
            }
        }
    }

    pthread_mutex_unlock(&params->input_lock);

    return NULL;
}

char *user_input_getline(char **input, const char *prompt, const sigint_action_t sigint_action)
{
    // Change SIGINT behavior
    if (sigint_action != sigint_action_dont_catch)
    {
        if (
        #ifdef __linux__
            signal(SIGINT, sigint_handler) == SIG_ERR
        #elif defined(_WIN32)
            !SetConsoleCtrlHandler(sigint_handler, TRUE)
        #endif
        ){
            perror(ANSI_RED "FATAL: Couldn't modify SIGINT handling");
            RESET_STDERR_COL();

            cleanup(NULL, NULL, NULL, sigint_action);

            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
    }

    struct routine_user_input_params params = { .input = *input };

    if (pthread_mutex_init(&params.input_lock, NULL))
    {
        perror(ANSI_RED "FATAL: Failed to initialize mutex");
        RESET_STDERR_COL();

        cleanup(NULL, NULL, params.input, sigint_action);

        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    pthread_t user_input_thread;
    if (pthread_create(&user_input_thread, NULL, (void *(*)(void *)) routine_user_input, &params))
    {
        perror(ANSI_RED "FATAL: Failed to spin up thread");
        RESET_STDERR_COL();

        cleanup(NULL, &params.input_lock, params.input, sigint_action);

        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    for (bool cont = true; cont; )
    {
        switch (pthread_mutex_trylock(&params.input_lock))
        {
            break; case EBUSY:
            {
                cont = false;
            }
            break; case 0:
            {
                if (pthread_mutex_unlock(&params.input_lock))
                {
                    perror(ANSI_RED "FATAL: Failed to unlock mutex from main");
                    RESET_STDERR_COL();

                    cleanup(&user_input_thread, &params.input_lock, params.input, sigint_action);

                    TF2_PLAYED_WITH_DEBUG_ABEX();
                }
            }
            break; default:
            {
                perror(ANSI_RED "FATAL: Misc mutex error");
                RESET_STDERR_COL();

                cleanup(&user_input_thread, &params.input_lock, params.input, sigint_action);

                TF2_PLAYED_WITH_DEBUG_ABEX();
            }
        }
    }

    putsnnl(prompt);

    while (1)
    {
        switch (pthread_mutex_trylock(&params.input_lock))
        {
            // Already locked
            break; case EBUSY:
            {
                if (caught_sigint)
                {
                    if (sigint_action)
                    {
                        sigint_action(prompt);
                    }

                    caught_sigint = false;
                }
            }
            // Got the lock successfully
            break; case 0:
            {
                *input = params.input;

                return (cleanup(&user_input_thread, &params.input_lock, NULL, sigint_action) ? NULL : params.input);
            }
            // Miscellaneous error
            break; default:
            {
                perror(ANSI_RED "FATAL: Misc mutex error");
                RESET_STDERR_COL();

                cleanup(&user_input_thread, &params.input_lock, params.input, sigint_action);

                TF2_PLAYED_WITH_DEBUG_ABEX();
            }
        }
    }

    // Should not get here
    TF2_PLAYED_WITH_DEBUG_ABEX();
}

bool user_input_confirm(const char *prompt, const sigint_action_t sigint_action)
{
    char *input = NULL;

    for (bool cont = true; cont; )
    {
        if (!user_input_getline(&input, prompt, sigint_action))
        {
            perror(ANSI_RED "FATAL: User confirmation input error");
            RESET_STDERR_COL();

            TF2_PLAYED_WITH_DEBUG_ABEX();
        }

        if (input[0] != '\0' && input[1] == '\0')
        {
            if (ccasecmp(input[0], 'Y'))
            {
                free(input);
                return true;
            }
            else if (ccasecmp(input[0], 'N'))
            {
                free(input);
                return false;
            }
        }
    }

    // Should not get here
    TF2_PLAYED_WITH_DEBUG_ABEX();
}
