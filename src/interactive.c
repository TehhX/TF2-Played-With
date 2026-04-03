#include "interactive.h"

#include "common.h"
#include "history.h"
#include "steamid_manip.h"
#include "collection.h"

#include "cider.h"

#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "pthread.h"

// Check if characters are equal, case agnostic. Assumes C2 is lowercase. Magic number is case bit/flag
#define ccasecmp(C1, C2) (((C1) | 0x20) == (C2))

// Test input against CMP and CMP[COFF]. If COFF is -1, no short version
#define INPUT_IS(INPUT, CMP, COFF) (!strncasecmp(INPUT, CMP, sizeof(CMP) - 1) || (COFF == -1 ? 0 : ccasecmp(INPUT[0], CMP[COFF])))

// Will print OUTPUT and if subsequent user input is positive, perform ACTION
HYPER_MACRO void interactive_action(const char *output, void (*action)())
{
    printf(ANSI_YELLOW "%s (Y/N) " ANSI_RESET, output);
    const int input = fgetc(stdin);
    if (ccasecmp(input, 'y') || input == '\n')
    {
        action();
    }

    if (input != '\n')
    {
        // MAJOR_TODO: When using option eg. load, Y/N prompt being given more than 1 character causes errant behavior. Should be fixed
        fgetc(stdin);
    }
}

// Parse SID3E from input_buf, perform action on it
HYPER_MACRO void perform_on_sid3e(char *input_buf, void (*action)(uint32_t sid3e))
{
    char *cursor, *specifier_start = NULL;
    for (cursor = input_buf; *cursor != '\n'; ++cursor)
    {
        if (!specifier_start && *cursor == ' ')
        {
            specifier_start = cursor + 1;
        }
    }
    if (!specifier_start)
    {
        fprintf(stderr, ANSI_RED "Forgot argument [SPEC]. Try 'help'.\n" ANSI_RESET);
        return;
    }
    *cursor = '\0';

    const uint32_t sid3e = sidm_parse_sid3e(specifier_start, Esteamid_type_unknown);

    if (sid3e == SIDM_ERR_NAME)
    {
        // MAJOR_TODO: Is name, need thing to do
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
        action(sid3e);
    }
}

HYPER_MACRO void action_delete_log()
{
    remove(history_live_log_path);
}

HYPER_MACRO void action_exit()
{
    exit(EXIT_SUCCESS);
}

// What tells the user to enter their input
#define USER_POKE "TF2PW > "

static pthread_t thread_collection;

void interactive_enter()
{
    printf("Interactive mode (try help):\n" USER_POKE);

    struct collection_read_live_routine_params live_params =
    {
        .continue_running = false,
    };

    // MAJOR_TODO: Pressing Ctrl+D in terminal while this reads will cause a seg fault (runtime error: load of null pointer of type 'char')
    for (char input_buf[STDIN_BUFB]; fgets(input_buf, STDIN_BUFB, stdin)[0]; printf(USER_POKE))
    {
        if (INPUT_IS(input_buf, "retrieve", 0))
        {
            perform_on_sid3e(input_buf, history_print_record);
        }
        else if (INPUT_IS(input_buf, "edit-notes", 5))
        {
            perform_on_sid3e(input_buf, history_edit_notes);
        }
        else if (INPUT_IS(input_buf, "collect-live", 10))
        {
            if (live_params.continue_running)
            {
                printf(ANSI_RED "Already live-collecting, can't start.\n" ANSI_RESET);
                continue;
            }

            printf("Starting live-collecting...\n");

            TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "Opening live-log \"%s\".\n" ANSI_RESET, history_live_log_path);

            FILE *const input_file_ptr = fopen(history_live_log_path, "r");
            if (!input_file_ptr)
            {
                perror(ANSI_RED "Failed to open live-file for reading. Error");
                SET_COLOR(stderr, ANSI_RESET);

                live_params.continue_running = false;

                continue;
            }

            live_params.continue_running = true;
            live_params.input_file = input_file_ptr;
            if (pthread_create(&thread_collection, NULL, collection_read_live_routine, &live_params))
            {
                fprintf(stderr, ANSI_RED "MAJOR: Failed to create thread_collection.\n" ANSI_RESET);
            }
            else
            {
                printf(ANSI_GREEN "Live collection started successfully.\n" ANSI_RESET);
            }
        }
        else if (INPUT_IS(input_buf, "stop-live", 1))
        {
            if (!live_params.continue_running)
            {
                printf(ANSI_RED "Not currently live-collecting, can't stop.\n" ANSI_RESET);

                continue;
            }

            printf("Stopping live-collection...\n");

            live_params.continue_running = false;
            if (pthread_join(thread_collection, NULL))
            {
                fprintf(stderr, ANSI_RED "MAJOR: Failed to join thread_collection.\n" ANSI_RESET);
            }
            else
            {
                printf(ANSI_GREEN "Live collection stopped successfully. Don't forget to save!\n" ANSI_RESET);
            }

            interactive_action("Delete live log file? (Must do before next live collection)", action_delete_log);

            if (fclose(live_params.input_file))
            {
                perror(ANSI_RED "Failed to close live-file. Error");
                SET_COLOR(stderr, ANSI_RESET);

                continue;
            }
        }
        else if (INPUT_IS(input_buf, "collect-archived", 8))
        {
            // Get start of specifier eg. "(r|retrieve) SPEC IF IER" -> "SPEC IF IER", replace '\n' with '\0'
            char *cursor, *specifier_start = NULL;
            for (cursor = input_buf; *cursor != '\n'; ++cursor)
            {
                if (!specifier_start && *cursor == ' ')
                {
                    specifier_start = cursor + 1;
                }
            }
            if (!specifier_start)
            {
                fprintf(stderr, ANSI_RED "Forgot argument [FULLNAME]. Try 'help'.\n" ANSI_RESET);
                continue;
            }
            *cursor = '\0';

            collection_read_archived(specifier_start);
        }
        // MAJOR_TODO: Read FULLNAME if provided
        else if (INPUT_IS(input_buf, "save", 0))
        {
            interactive_action("Overwrite file and save?", history_save);
        }
        // MAJOR_TODO: Read FULLNAME if provided
        else if (INPUT_IS(input_buf, "load", 0))
        {
            interactive_action("Overwrite current data and load?", history_load);
        }
        else if (INPUT_IS(input_buf, "help", 0))
        {
            printf
            (
                ANSI_BLUE
                    LTAB "TF2PW Interactive Mode Help | Try any below phrase or the enclosed character (eg. retrieve = r) (case insensitive)\n"
                    LTAB LTAB "(r)etrieve [STEAMID3|STEAMID3E|STEAMID64|NAME]\n"
                    LTAB LTAB LTAB "Retrieve and print associated record.\n"
                    LTAB LTAB "edit-(n)otes [STEAMID3|STEAMID3E|STEAMID64|NAME]\n"
                    LTAB LTAB LTAB "Open your $EDITOR (or vi if none provided) to edit notes for the specified player.\n"
                    LTAB LTAB "collect-li(v)e [?FULLNAME]\n"
                    LTAB LTAB LTAB "Collects live data from FULLNAME. If FULLNAME not provided, collect from saved path.\n"
                    LTAB LTAB "s(t)op-live\n"
                    LTAB LTAB LTAB "Stops collecting live data.\n"
                    LTAB LTAB "collect-(a)rchived [FULLNAME]\n"
                    LTAB LTAB LTAB "Collects data from an already completed log file located at FULLNAME.\n"
                    LTAB LTAB "(s)ave [?FULLNAME]\n"
                    LTAB LTAB LTAB "Save records to history file. If no FULLNAME provided, use default.\n"
                    LTAB LTAB "(l)oad [?FULLNAME]\n"
                    LTAB LTAB LTAB "Load records from history file. If no FULLNAME provided, use default. IMPORTANT: Remember to load before manipulating/reading history.\n"
                    LTAB LTAB "(h)elp\n"
                    LTAB LTAB LTAB "Display this help message.\n"
                    LTAB LTAB "(e)xit|(q)uit\n"
                    LTAB LTAB LTAB "Exit interactive mode. Will ask if you want to save first, then confirm.\n"
                    LTAB LTAB "force-exit|force-quit\n"
                    LTAB LTAB LTAB "Forcefully exit. Will not confirm, or ask to save first. May break save file, only slightly preferable to Ctrl+C-ing TF2PW.\n"
                    LTAB LTAB "(c)lear\n"
                    LTAB LTAB LTAB "Clear the terminal (Terminal dependent).\n"
                ANSI_RESET
            );
        }
        else if (INPUT_IS(input_buf, "exit", 0) || INPUT_IS(input_buf, "quit", 0))
        {
            interactive_action("Save before quitting?", history_save);

            interactive_action("Really exit?", action_exit);
        }
        else if (INPUT_IS(input_buf, "force-exit", -1) || INPUT_IS(input_buf, "force-quit", -1))
        {
            return;
        }
        else if (INPUT_IS(input_buf, "clear", 0))
        {
            system("clear");
        }
        // NEWARGS_TODO
        else if (input_buf[0] != '\n')
        {
            printf(ANSI_RED "Bad input, try again.\n" ANSI_RESET);
        }
    }
}
