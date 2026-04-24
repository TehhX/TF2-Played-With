#include "interactive.h"

#include "common.h"
#include "history.h"
#include "steamid_manip.h"
#include "collection.h"
#include "user_input.h"

#include "cider.h"

#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "pthread.h"

// Change definition of strncasecmp if on Windows
#if defined(_WIN32) || defined(_WIN64)
    #define strncasecmp _strnicmp
#endif

// Test input against CMP and CHAR. If COFF is -1, no short version
#define INPUT_IS(CMP, CHAR) (!strncasecmp(input_buf, CMP, sizeof(CMP) - 1) || (((char) (CHAR)) == ((char) (-1)) ? 0 : (ccasecmp(input_buf[0], CHAR)) && (input_buf[1] == '\0' || input_buf[1] == ' ')))

enum Especifier_status
{
    Especifier_status_invocation,     // Eg. retrieve
    Especifier_status_pre_specifier,  // Whitespace between invocation and specifier
    Especifier_status_specifier,      // Inside specifier
    Especifier_status_post_specifier, // Testing whitespace after specifier until null-terminator
};

/*
    @brief Get specifier start

        @param input_buf The input buffer from which to retrieve the specifier
        @param complain_if_no_param If true, complain about there being no parameter, else stay quiet
*/
static char *get_spec_start(char *input_buf, bool complain_if_no_param)
{
    enum Especifier_status curr_status = Especifier_status_invocation;

    char *specifier_start, *end_whitespace;
    for (char *cursor = input_buf + 1; true; ++cursor)
    {
        switch (curr_status)
        {
            break; case Especifier_status_invocation:
            {
                if (*cursor == ' ')
                {
                    curr_status = Especifier_status_pre_specifier;
                }
                else if (*cursor == '\0')
                {
                    goto SPEC_FAILURE;
                }
            }
            break; case Especifier_status_pre_specifier:
            {
                if (*cursor != ' ')
                {
                    specifier_start = cursor;
                    curr_status = Especifier_status_specifier;
                }
                else if (*cursor == '\0')
                {
                    goto SPEC_FAILURE;
                }
            }
            break; case Especifier_status_specifier:
            {
                if (*cursor == ' ')
                {
                    curr_status = Especifier_status_post_specifier;
                    end_whitespace = cursor;
                }
                else if (*cursor == '\0')
                {
                    return specifier_start;
                }
            }
            break; case Especifier_status_post_specifier:
            {
                switch (*cursor)
                {
                    break; case ' ':
                    {
                        continue;
                    }
                    break; case '\0':
                    {
                        *end_whitespace = '\0';
                        return specifier_start;
                    }
                    break; default:
                    {
                        curr_status = Especifier_status_specifier;
                    }
                }
            }
        }
    }

    SPEC_FAILURE:;
    if (complain_if_no_param)
    {
        fputs(ANSI_RED "Failed to get start of specifier.\n" ANSI_RESET, stderr);
    }

    return NULL;
}

// @brief Parse SID3E from input_buf, perform action on it
static void perform_on_sid3e(char *input_buf, void (*sid3e_action)(uint32_t sid3e), void (*specifier_action)(const char *specifier))
{
    char *const specifier_start = get_spec_start(input_buf, true);
    if (!specifier_start)
    {
        return;
    }

    const uint32_t sid3e = sidm_parse_sid3e(specifier_start, Esteamid_type_unknown);

    if (sid3e == SIDM_ERR_NAME)
    {
        if (specifier_action)
        {
            specifier_action(specifier_start);
        }
        else
        {
            fputs(ANSI_RED "MAJOR: Parsed name for option without associated action.\n" ANSI_RESET, stderr);
            return;
        }
    }
    else if (sid3e == SIDM_ERR_RNGE)
    {
        fprintf(stderr, ANSI_RED "ID value too large. Try again.\n" ANSI_RESET);
    }
    else if (sid3e == SIDM_ERR_MISC)
    {
        fprintf(stderr, ANSI_RED "Bad ID value. Try again.\n" ANSI_RESET);
    }
    else
    {
        sid3e_action(sid3e);
    }
}

static void sigint_action_warn(const char *prompt)
{
    fputs(ANSI_RED " | Caught SIGINT, use \"(q)uit\" to quit.\n" ANSI_RESET, stderr);

    fputs(prompt, stdout);
    fflush(stdout);
}

static void sigint_action_reprint(const char *prompt)
{
    printf("\n%s", prompt);
    fflush(stdout);
}

void interactive_enter()
{
    puts("Interactive mode (try help):");

    pthread_t thread_collection;
    char *user_input_history_fullname = NULL;

    struct collection_read_live_routine_params live_params = { .input_file = NULL, .running = false };

    char *input_buf = NULL;

    while (1)
    {
        if (!user_input_getline(&input_buf, "TF2PW > ", sigint_action_warn))
        {
            perror(ANSI_RED "FATAL: Couldn't get user input.");
            RESET_STDERR_COL();

            TF2_PLAYED_WITH_DEBUG_ABEX();
        }

        if (INPUT_IS("retrieve", 'r'))
        {
            perform_on_sid3e(input_buf, history_print_record, history_print_records);
        }
        else if (INPUT_IS("set-tf2-filepath", 'p'))
        {
            char *const specifier_start = get_spec_start(input_buf, true);
            if (specifier_start)
            {
                char *const specifier_start_heap = string_deep_copy(specifier_start);
                history_set_tf2_filepath(specifier_start_heap);

                printf("Successfully set TF2 filepath to \"%s\".\n", specifier_start);
            }
        }
        else if (INPUT_IS("edit-notes", 'n'))
        {
            perform_on_sid3e(input_buf, history_edit_notes, NULL);
        }
        else if (INPUT_IS("collect-live", 'v'))
        {
            if (live_params.running)
            {
                printf(ANSI_RED "Already live-collecting, can't start.\n" ANSI_RESET);
                continue;
            }

            puts("Starting live-collecting...");

            TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "Opening live-log \"%s\".\n" ANSI_RESET, history_get_live_log_fullname());

            FILE *const input_file_ptr = fopen(history_get_live_log_fullname(), "r");
            if (!input_file_ptr)
            {
                perror(ANSI_RED "Failed to open live-file for reading. Error");
                RESET_STDERR_COL();

                continue;
            }

            live_params.input_file = input_file_ptr;
            if (pthread_create(&thread_collection, NULL, collection_read_live_routine, &live_params))
            {
                perror(ANSI_RED "MAJOR: Failed to create thread_collection");
                RESET_STDERR_COL();
            }
            else
            {
                puts(ANSI_GREEN "Live collection started successfully." ANSI_RESET);
            }
        }
        else if (INPUT_IS("stop-live", 't'))
        {
            if (!live_params.running)
            {
                fprintf(stderr, ANSI_RED "Not currently live-collecting, can't stop.\n" ANSI_RESET);

                continue;
            }

            printf("Stopping live-collection...\n");
            live_params.running = false;

            if (pthread_join(thread_collection, NULL))
            {
                perror(ANSI_RED "MAJOR: Failed to join thread_collection");
                RESET_STDERR_COL();
            }
            else
            {
                puts(ANSI_GREEN "Live collection stopped successfully. Don't forget to save!" ANSI_RESET);
            }

            if (user_input_confirm(ANSI_YELLOW "Delete live log file? (Must do before next live collection) (Y/N): " ANSI_RESET, sigint_action_reprint))
            {
                if (remove(history_get_live_log_fullname()))
                {
                    fprintf(stderr, ANSI_RED "Failed to remove log file \"%s\", manual intervention required before next live collection: ", history_get_live_log_fullname());
                    perror(NULL);
                    RESET_STDERR_COL();
                }
                else
                {
                    printf(ANSI_GREEN "Successfully removed log file \"%s\".\n" ANSI_RESET, history_get_live_log_fullname());
                }
            }

            if (fclose(live_params.input_file))
            {
                perror(ANSI_RED "Failed to close live-file. Error");
                RESET_STDERR_COL();

                continue;
            }
        }
        else if (INPUT_IS("collect-archived", 'a'))
        {
            const char *const specifier_start = get_spec_start(input_buf, true);

            if (specifier_start)
            {
                collection_read_archived(specifier_start);
            }
        }
        else if (INPUT_IS("save", 's'))
        {
            const char *const specifier_start = get_spec_start(input_buf, false);

            if (specifier_start)
            {
                user_input_history_fullname = string_deep_copy(specifier_start);
            }
            else
            {
                free(user_input_history_fullname);
                user_input_history_fullname = NULL;
            }

            if (user_input_confirm(ANSI_YELLOW "Overwrite file and save? (Y/N): " ANSI_RESET, sigint_action_reprint))
            {
                history_save(user_input_history_fullname);
            }
        }
        else if (INPUT_IS("load", 'l'))
        {
            const char *const specifier_start = get_spec_start(input_buf, false);

            if (specifier_start)
            {
                user_input_history_fullname = string_deep_copy(specifier_start);
            }
            else
            {
                free(user_input_history_fullname);
                user_input_history_fullname = NULL;
            }

            if (user_input_confirm(ANSI_YELLOW "Discard changes in memory and load? (Y/N): " ANSI_RESET, sigint_action_reprint))
            {
                history_load(user_input_history_fullname);
            }
        }
        else if (INPUT_IS("help", 'h'))
        {
            printf
            (
                ANSI_BLUE
                    LTAB "TF2PW Interactive Mode Help | Try any below phrase or the enclosed character (eg. retrieve = r) (case insensitive)\n"
                    LTAB LTAB "(r)etrieve [STEAMID3|STEAMID3E|STEAMID64|NAME]\n"
                    LTAB LTAB LTAB "Retrieve and print associated record.\n\n"
                    LTAB LTAB "set-tf2-file(p)ath [FILEPATH]\n"
                    LTAB LTAB LTAB "Sets the filepath of your TF2 directory. Should follow the form \".../Team Fortress 2/\".\n\n"
                    LTAB LTAB "edit-(n)otes [STEAMID3|STEAMID3E|STEAMID64|NAME]\n"
                    LTAB LTAB LTAB "Open your $EDITOR (or vi if none provided) to edit notes for the specified player.\n\n"
                    LTAB LTAB "collect-li(v)e\n"
                    LTAB LTAB LTAB "Collects live data from saved path.\n\n"
                    LTAB LTAB "s(t)op-live\n"
                    LTAB LTAB LTAB "Stops collecting live data.\n\n"
                    LTAB LTAB "collect-(a)rchived [FULLNAME]\n"
                    LTAB LTAB LTAB "Collects data from an already completed log file located at FULLNAME.\n\n"
                    LTAB LTAB "(s)ave [?FULLNAME]\n"
                    LTAB LTAB LTAB "Save records to history file. If no FULLNAME provided, use default.\n\n"
                    LTAB LTAB "(l)oad [?FULLNAME]\n"
                    LTAB LTAB LTAB "Load records from history file. If no FULLNAME provided, use default. IMPORTANT: Remember to load before manipulating/reading history.\n\n"
                    LTAB LTAB "(h)elp\n"
                    LTAB LTAB LTAB "Display this help message.\n\n"
                    LTAB LTAB "(e)xit|(q)uit\n"
                    LTAB LTAB LTAB "Exit interactive mode. Will ask if you want to save first, then confirm.\n\n"
                    LTAB LTAB "(c)lear\n"
                    LTAB LTAB LTAB "Clear the terminal (Terminal dependent).\n"
                ANSI_RESET
            );
        }
        else if (INPUT_IS("exit", 'e') || INPUT_IS("quit", 'q'))
        {
            if (live_params.running)
            {
                printf(ANSI_RED "Can't quit while live-collecting.\n" ANSI_RESET);

                continue;
            }

            if (user_input_confirm(ANSI_YELLOW "Save before quitting? (Y/N): " ANSI_RESET, sigint_action_reprint))
            {
                history_save(user_input_history_fullname);
            }

            if (user_input_confirm(ANSI_YELLOW "Really quit? (Y/N): " ANSI_RESET, sigint_action_reprint))
            {
                break;
            }
        }
        else if (INPUT_IS("clear", 'c'))
        {
            #ifdef __linux__
                system("clear");
            #elif defined(_WIN32)
                system("cls");
            #endif
        }
        // NEWARGS_TODO
        else
        {
            printf(ANSI_RED "Bad input \"%s\", try again.\n" ANSI_RESET, input_buf);
        }
    }

    free(input_buf);
    free(user_input_history_fullname);
}
