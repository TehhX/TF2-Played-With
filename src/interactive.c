#include "interactive.h"

#include "common.h"
#include "history.h"
#include "steamid_manip.h"

#include "stdio.h"
#include "string.h"
#include "ctype.h"

// Check if characters are equal, case agnostic. Assumes C2 is lowercase. Magic number is capitalization bit
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
            // IMPL_TODO
        }
        else if (INPUT_IS("stop-live", 1))
        {
            // IMPL_TODO
        }
        else if (INPUT_IS("save", 0))
        {
            interactive_action("Overwrite file and save?", history_save());
        }
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
                    LITERAL_TAB LITERAL_TAB "- (r)etrieve [SPEC]: Retrieve and print record using specifier STEAMID3, STEAMID64, or NAME\n"
                    LITERAL_TAB LITERAL_TAB "-    collect-li(v)e: Collects live data \n"
                    LITERAL_TAB LITERAL_TAB "-       s(t)op-live: Stops collecting live data\n"
                    LITERAL_TAB LITERAL_TAB "-            (s)ave: Save records to history file\n"
                    LITERAL_TAB LITERAL_TAB "-            (l)oad: Load records from history file. IMPORTANT: Remember to load before manipulating/reading history\n"
                    LITERAL_TAB LITERAL_TAB "-            (h)elp: Display this help message\n"
                    LITERAL_TAB LITERAL_TAB "-            (e)xit: Exit interactive mode. Will ask if you want to save first, then confirm\n"
                    LITERAL_TAB LITERAL_TAB "-        force-exit: Forcefully exit. Will not confirm, or ask to save first. May break save file, only slightly preferable to Ctrl+C-ing TF2PW\n"
                    LITERAL_TAB LITERAL_TAB "-           (c)lear: Clear the terminal (Terminal dependent)\n"
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
        // Unknown input (just enter is allowed)
        else if (input_buf[0] != '\n')
        {
            printf(ANSI_RED "Bad input, try again.\n" ANSI_RESET, input_buf);
        }
    }
}
