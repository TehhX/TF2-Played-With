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
#define ccasecmp(C1, C2) (((C1) | 0b00100000) == (C2))

#define interactive_action(OUTPUT, ACTION)\
{\
    printf(ANSI_YELLOW OUTPUT " (Y/N) " ANSI_RESET);\
    const int input = fgetc(stdin);\
    if (ccasecmp(input, 'y') || input == '\n')\
    {\
        ACTION;\
    }\
\
    if (input != '\n')\
    {\
        /* MAJOR_TODO: When using option eg. load, Y/N prompt being given more than 1 character causes errant behavior. Should be fixed */\
        fgetc(stdin);\
    }\
}

// What tells the user to enter their input
#define USER_POKE "TF2PW > "

// Size of stdin buffer in bytes
#define STDIN_BUFB 128

// Test input against CMP and CMP[COFF]. If COFF is -1, no short version
#define INPUT_IS(CMP, COFF) (!strncasecmp(input_buf, CMP, sizeof(CMP) - 1) || (COFF == -1 ? 0 : ccasecmp(input_buf[0], CMP[COFF])))

static pthread_t thread_collection;

static struct collection_read_live_routine_params live_params =
{
    .continue_running = false,
    .collection_fullname = NULL
};

void interactive_enter()
{
    printf("Interactive mode (try help):\n" USER_POKE);

    // MAJOR_TODO: Pressing Ctrl+D in terminal while this reads will cause a seg fault (runtime error: load of null pointer of type 'char')
    for (char input_buf[STDIN_BUFB]; fgets(input_buf, STDIN_BUFB, stdin)[0]; printf(USER_POKE))
    {
        if (INPUT_IS("retrieve", 0))
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
                fprintf(stderr, ANSI_RED "Forgot argument [SPEC]. Try 'help'.\n" ANSI_RESET);
                continue;
            }
            *cursor = '\0';

            const uint32_t sid3e = sidm_parse_sid3e(specifier_start, Esteamid_type_unknown);

            // Assuming name due to misc parsing error
            if (sid3e == SIDM_ERR_MISC)
            {
                history_print_records(specifier_start);
            }
            else if (sid3e == SIDM_ERR_RNGE)
            {
                printf(ANSI_RED "ID value too large. Try again.\n" ANSI_RESET);
            }
            else
            {
                history_print_record(sid3e);
            }
        }
        else if (INPUT_IS("collect-live", 10))
        {
            if (live_params.continue_running)
            {
                printf(ANSI_RED "Already live-collecting, can't start.\n" ANSI_RESET);
                continue;
            }

            printf("Starting live-collecting...\n");

            live_params.continue_running = true;
            if (pthread_create(&thread_collection, NULL, collection_read_live_routine, &live_params))
            {
                fprintf(stderr, ANSI_RED "MAJOR: Failed to create thread_collection.\n" ANSI_RESET);
            }
            else
            {
                printf(ANSI_GREEN "Live collection started successfully.\n" ANSI_RESET);
            }
        }
        else if (INPUT_IS("stop-live", 1))
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
        }
        // MAJOR_TODO: Read FULLNAME if provided
        else if (INPUT_IS("save", 0))
        {
            interactive_action("Overwrite file and save?", history_save());
        }
        // MAJOR_TODO: Read FULLNAME if provided
        else if (INPUT_IS("load", 0))
        {
            interactive_action("Overwrite current data and load?", history_load());
        }
        else if (INPUT_IS("help", 0))
        {
            printf
            (
                ANSI_BLUE
                    LITERAL_TAB "TF2PW Interactive Mode Help | Try any below phrase or the enclosed character (eg. retrieve = r) (case insensitive)\n"
                    LITERAL_TAB LITERAL_TAB "(r)etrieve [STEAMID3|STEAMID3E|STEAMID64|NAME]\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Retrieve and print associated record.\n"
                    LITERAL_TAB LITERAL_TAB "collect-li(v)e [FULLNAME]\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Collects live data.\n"
                    LITERAL_TAB LITERAL_TAB "s(t)op-live\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Stops collecting live data.\n"
                    LITERAL_TAB LITERAL_TAB "(s)ave [?FULLNAME]\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Save records to history file. If no FULLNAME provided, use default.\n"
                    LITERAL_TAB LITERAL_TAB "(l)oad [?FULLNAME]\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Load records from history file. If no FULLNAME provided, use default. IMPORTANT: Remember to load before manipulating/reading history.\n"
                    LITERAL_TAB LITERAL_TAB "(h)elp\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Display this help message.\n"
                    LITERAL_TAB LITERAL_TAB "(e)xit\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Exit interactive mode. Will ask if you want to save first, then confirm.\n"
                    LITERAL_TAB LITERAL_TAB "force-exit\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Forcefully exit. Will not confirm, or ask to save first. May break save file, only slightly preferable to Ctrl+C-ing TF2PW.\n"
                    LITERAL_TAB LITERAL_TAB "(c)lear\n"
                    LITERAL_TAB LITERAL_TAB LITERAL_TAB "Clear the terminal (Terminal dependent).\n"
                ANSI_RESET
            );
        }
        else if (INPUT_IS("exit", 0))
        {
            interactive_action("Overwrite file and save?", history_save());

            interactive_action("Really exit?", return);
        }
        else if (INPUT_IS("force-exit", -1))
        {
            return;
        }
        else if (INPUT_IS("clear", 0))
        {
            system("clear");
        }
        else if (input_buf[0] != '\n')
        {
            printf(ANSI_RED "Bad input, try again.\n" ANSI_RESET, input_buf);
        }
    }
}
