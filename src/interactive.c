#include "interactive.h"

#include "common.h"
#include "history.h"

#include "stdio.h"
#include "string.h"
#include "ctype.h"

// Check if characters are equal, case agnostic. Assumes C2 is lowercase
#define ccasecmp(C1, C2) ((isupper(C1) ? (C1) - ('A' - 'a') : C1) == (C2))

// Will ask the user for confirmation before calling history_save()
HYPER_MACRO void interactive_save()
{
    printf(ANSI_YELLOW "Save? (Y/N) " ANSI_RESET);
    const int input = fgetc(stdin);
    if (ccasecmp(input, 'y') || input == '\n')
    {
        history_save();
    }

    if (input != '\n')
    {
        // FGETC_TODO
        fgetc(stdin);
    }
}

// Will ask the user for confirmation before calling history_load()
HYPER_MACRO void interactive_load()
{
    printf(ANSI_YELLOW "Load? (Y/N) " ANSI_RESET);
    const int input = fgetc(stdin);
    if (ccasecmp(input, 'y') || input == '\n')
    {
        history_load();
    }

    if (input != '\n')
    {
        // FGETC_TODO
        fgetc(stdin);
    }
}

// What tells the user to enter their input
#define USER_POKE "TF2PW > "

// Size of stdin buffer in bytes
#define STDIN_BUFB 128

// Test input against CMP and the first letter of CMP
#define INPUT_IS(CMP) (!strncasecmp(input_buf, CMP, sizeof(CMP) - 1) || ccasecmp(input_buf[0], CMP[0]))

void interactive_enter()
{
    printf("Interactive mode:\n" USER_POKE);

    // MAJOR_TODO: Pressing Ctrl+D in terminal while this reads will cause a seg fault for some reason
    for (char input_buf[STDIN_BUFB]; fgets(input_buf, STDIN_BUFB, stdin)[0]; printf(USER_POKE))
    {
        if (INPUT_IS("save"))
        {
            interactive_save();
        }
        else if (INPUT_IS("load"))
        {
            interactive_load();
        }
        else if (INPUT_IS("help"))
        {
            printf
            (
                LITERAL_TAB ANSI_BLUE "TF2PW Interactive Mode Help\n"
                LITERAL_TAB LITERAL_TAB "- (s)ave:  Save memory to disk\n"
                LITERAL_TAB LITERAL_TAB "- (l)oad:  Load disk to memory\n"
                LITERAL_TAB LITERAL_TAB "- (h)elp:  Display this message\n"
                LITERAL_TAB LITERAL_TAB "- (q)uit:  Quit interactive mode\n"
                LITERAL_TAB LITERAL_TAB "- (c)lear: Clear the terminal\n"
                ANSI_RESET
            );
        }
        else if (INPUT_IS("quit"))
        {
            printf(ANSI_YELLOW "Quit? (Y/N) " ANSI_RESET);
            const int input = fgetc(stdin);
            if (ccasecmp(input, 'y') || input == '\n')
            {
                if (input != '\n')
                {
                    // FGETC_TODO
                    fgetc(stdin);
                }

                interactive_save();
                return;
            }

            if (input != '\n')
            {
                // FGETC_TODO
                fgetc(stdin);
            }
        }
        else if (INPUT_IS("clear"))
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
