#include "user_input.h"

#include "common.h"

#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "errno.h"

enum bad_input_type
{
    bad_input_type_none,
    bad_input_type_sigint,
    bad_input_type_break
};

static enum bad_input_type bad_input_type = bad_input_type_none;

#ifdef __linux__
    #include "signal.h"

    #include "readline/readline.h"

    void sigint_handler(const int sig_code)
    {
        bad_input_type = bad_input_type_sigint;
    }
#elif defined(_WIN32)
    #include "windows.h"

    BOOL WINAPI sigint_handler(const DWORD sig_code)
    {
        bad_input_type = bad_input_type_sigint;

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
    #ifdef _WIN32
        size_t input_len;
    #endif

    const char *const prompt;
    char *input;
    pthread_mutex_t input_lock;
};

static void *routine_user_input(struct routine_user_input_params *const params)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        // Should be pre-locked by user_input_getline(...)
        if (pthread_mutex_trylock(&params->input_lock) != EBUSY)
        {
            fputs(ANSI_RED "FATAL: Mutex not pre-locked in routine_user_input.\n" ANSI_RESET, stderr);
            abort();
        }
    )

    #ifdef __linux__
        free(params->input);
        if ((params->input = readline(params->prompt)) == NULL)
        {
            bad_input_type = bad_input_type_break;

            return NULL;
        }
    #elif defined(_WIN32)
        fputs(params->prompt, stdout);
        fflush(stdout);

        params->input_len = 0;

        for (bool cont = true; cont; )
        {
            // BUFF_TODO
            const int next = fgetc(stdin);
            switch (next)
            {
                break; case '\n':
                {
                    prealloc(params->input, sizeof(char), ++params->input_len);
                    params->input[params->input_len - 1] = '\0';

                    cont = false;
                }
                break; case EOF:
                {
                    params->input_len = 0;
                }
                break; default:
                {
                    prealloc(params->input, sizeof(char), params->input_len + 1);
                    params->input[params->input_len++] = (char) next;
                }
            }
        }
    #endif

    pthread_mutex_unlock(&params->input_lock);

    return NULL;
}

char *user_input_getline(char **input, const char *prompt, const char *bad_input_message)
{
    const bool catch_sigint = bad_input_message != NULL;

    // Change SIGINT behavior
    if (catch_sigint)
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

            cleanup(NULL, NULL, NULL, catch_sigint);

            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
    }

    struct routine_user_input_params params = { .input = *input, .prompt = prompt };

    if (pthread_mutex_init(&params.input_lock, NULL))
    {
        perror(ANSI_RED "FATAL: Failed to initialize mutex");
        RESET_STDERR_COL();

        cleanup(NULL, NULL, params.input, catch_sigint);

        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    // Pre-lock for routine's sake
    pthread_mutex_lock(&params.input_lock);

    pthread_t user_input_thread;
    if (pthread_create(&user_input_thread, NULL, (void *(*)(void *)) routine_user_input, &params))
    {
        perror(ANSI_RED "FATAL: Failed to spin up thread");
        RESET_STDERR_COL();

        cleanup(NULL, &params.input_lock, params.input, catch_sigint);

        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    while (1)
    {
        if (bad_input_type != bad_input_type_none)
        {
            if (bad_input_type == bad_input_type_sigint)
            {
                fprintf(stderr, "\n%s\n", bad_input_message);
            }
            else if (bad_input_type == bad_input_type_break)
            {
                fprintf(stderr, "%s\n", bad_input_message);
            }

            // Disregard old input if using readline(...)
            #ifdef __linux__
                pthread_cancel(user_input_thread);

                // Initialize mutex, pre-lock
                pthread_mutex_destroy(&params.input_lock);
                pthread_mutex_init(&params.input_lock, NULL);
                pthread_mutex_lock(&params.input_lock);

                params.input = NULL;

                pthread_create(&user_input_thread, NULL, (void *(*)(void *)) routine_user_input, &params);
            #elif defined(_WIN32)
                fputs(prompt, stdout);
                fflush(stdout);
            #endif

            bad_input_type = bad_input_type_none;
        }

        switch (pthread_mutex_trylock(&params.input_lock))
        {
            // Already locked
            break; case EBUSY:
            {
                // No-op
            }
            // Got the lock successfully
            break; case 0:
            {
                *input = params.input;

                return (cleanup(&user_input_thread, &params.input_lock, NULL, catch_sigint) ? NULL : *input);
            }
            // Miscellaneous error
            break; default:
            {
                perror(ANSI_RED "FATAL: Misc mutex error");
                RESET_STDERR_COL();

                cleanup(&user_input_thread, &params.input_lock, params.input, catch_sigint);

                TF2_PLAYED_WITH_DEBUG_ABEX();
            }
        }
    }

    // Should not get here
    TF2_PLAYED_WITH_DEBUG_ABEX();
}

bool user_input_confirm(const char *prompt, const char *bad_input_message)
{
    char *input = NULL;

    for (bool cont = true; cont; )
    {
        if (!user_input_getline(&input, prompt, bad_input_message))
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
