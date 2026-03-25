#include "collection.h"

#include "common.h"
#include "history.h"
#include "player_info.h"

#include "stdio.h"
#include "string.h"
#include "inttypes.h"

void collection_read_live(const char *collection_fullname)
{
    // TODO: Implement
    // TODO: Set date via history_set_date(...) before adding records via history_add_record(...) every time, program may be run across multiple days
}

void collection_read_archived(const char *collection_fullname)
{
    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error: ", collection_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: Reading archive log file \"%s\".\n", collection_fullname);

    // TODO: Set history current_date to creation date of collection_fullname. Just using date of run for now
    history_set_date(HISTORY_SET_DATE_TODAY);

    steam_name_stack user_name = { '\0' };

    // How many players can be in a match (including user)
    #define MAX_PLAYERS 128

    // Hold statuses of all players
    int player_info_arr_len;
    struct player_info player_info_arr[MAX_PLAYERS];

    // Size of the line buffer in bytes. Should be a couple bytes larger than the largest expected line length. Keep in mind that values via pow(2, (int) exp) eg. 512 make it seem up to 8 times more professional
    #define LINE_BUFB 256

    // REMINDER: End of line will be '\n', not '\0'
    TF2_PLAYED_WITH_DEBUG_INSERT(size_t line_index = 1; int match_index = (int) line_index;)
    for (char line_buf[LINE_BUFB]; fgets(line_buf, LINE_BUFB, input_file_ptr); TF2_PLAYED_WITH_DEBUG_INSERT(++line_index))
    {
        // All status lines containing a name-sid3e have 2 spaces after the octothorpe, title only has 1
        #define STATUS_PREFIX (char [3]){ "#  " }

        // Check for status output
        // TODO: Doesn't check that line is long enough. Probably not an issue, but keep testing for it and remove this comment when it's sure to cause no problems
        int user_name_len;
        if (!memcmp(line_buf, STATUS_PREFIX, sizeof(STATUS_PREFIX)))
        {
            int line_i = sizeof(STATUS_PREFIX), player_i = 0;

            // Get player name start index
            while (line_buf[line_i++] != '"');
            const int player_name_begin = line_i;

            // This is user, skip
            if (!strncmp(user_name, line_buf + player_name_begin, user_name_len))
            {
                goto STATUS_FINISH;
            }

            // Get index of last closing bracket
            int last_close_bracket_i = 0, last_bot_str_i = 0;
            while (line_buf[++line_i] != '\n')
            {
                if (line_buf[line_i] == ']')
                {
                    last_close_bracket_i = line_i;
                }
                else if (line_buf[line_i] == 'B' && line_buf[line_i + 1] == 'O' && line_buf[line_i + 2] == 'T')
                {
                    last_bot_str_i = line_i;
                }
            }

            // This is a bot, skip
            if (last_bot_str_i >= last_close_bracket_i)
            {
                TF2_PLAYED_WITH_DEBUG_INSERT(printf("LOG: (LI=%zu) Bot found, skipping.\n", line_index);)
                goto STATUS_FINISH;
            }

            // Set line_i to index of start of SID3E
            for (line_i = last_close_bracket_i; line_buf[--line_i - 1] != ':'; );

            char *end;
            const uint32_t current_sid3e = strtol(line_buf + line_i, &end, 10);
            if (*end != ']')
            {
                TF2_PLAYED_WITH_DEBUG_CHOOSE
                (
                    fprintf(stderr, "FATAL: (LI=%zu) Bad current_sid3e read from status output.\n", line_index);
                    ,
                    fputs("FATAL: Bad current_sid3e read from status output.\n", stderr);
                )

                TF2_PLAYED_WITH_DEBUG_ABEX();
            }

            // If current SID3E is not found in arr
            #define CURRENT_NOT_FOUND ((int) -1)

            // BSEARCH_TODO
            int current_arr_index = CURRENT_NOT_FOUND;
            for (int i = 0; i < player_info_arr_len; ++i)
            {
                if (player_info_arr[i].sid3e == current_sid3e)
                {
                    current_arr_index = i;
                    break;
                }
            }

            // Player not in status array, add them
            if (current_arr_index == CURRENT_NOT_FOUND)
            {
                current_arr_index = player_info_arr_len++;

                // Get player name end index
                while (line_buf[--line_i] != '"');
                const int player_name_len = line_i - player_name_begin;

                // Set name and SID3E in player_info_arr
                memcpy(player_info_arr[current_arr_index].name, line_buf + player_name_begin, player_name_len);
                player_info_arr[current_arr_index].name[player_name_len] = '\0';
                player_info_arr[current_arr_index].sid3e                 = current_sid3e;

                // A million of these lines will be printed for an average log file, may or may not be commented out for any given commit
                // TF2_PLAYED_WITH_DEBUG_INSERT(printf("LOG: (LI=%llu) Status line of new player #%3d in match #%2zu parsed: (\"%s\", %" PRIu32 ")\n", line_index, current_arr_index + 1, match_index, player_info_arr[current_arr_index].name, player_info_arr[current_arr_index].sid3e);)
            }
        }
        STATUS_FINISH:

        // Check for connected message in the form `<NAME> connected`
        #define CONNECTED_SUFFIX (char [9]){ "connected" }
        for (int i = 0; i < LINE_BUFB; ++i)
        {
            if (line_buf[i] == '\n')
            {
                // Line is not long enough to house connected statement, checking may cause a program crash and is pointless anyway
                if (i <= sizeof(CONNECTED_SUFFIX))
                {
                    goto CONNECTED_FINISH;
                }

                if (!memcmp(line_buf + i - sizeof(CONNECTED_SUFFIX), CONNECTED_SUFFIX, sizeof(CONNECTED_SUFFIX)))
                {
                    const int name_end = i - sizeof(CONNECTED_SUFFIX) - 1;

                    // Is first occurrence of username
                    if (user_name[0] == '\0')
                    {
                        memcpy(user_name, line_buf, name_end);
                        user_name[name_end] = '\0';
                        user_name_len = name_end;

                        player_info_arr_len = 0;

                        TF2_PLAYED_WITH_DEBUG_INSERT(printf("LOG: (LI=%llu) First occurrence of username connected: \"%s\"\n", line_index, user_name);)
                    }
                    // Is another occurrence of username
                    else if (!memcmp(line_buf, user_name, name_end))
                    {
                        TF2_PLAYED_WITH_DEBUG_INSERT(++match_index;)

                        TF2_PLAYED_WITH_DEBUG_INSERT(printf("LOG: (LI=%llu) Another occurrence of username connected: \"%.*s\"\n", line_index, name_end, line_buf);)

                        for (int i = 0; i < player_info_arr_len; ++i)
                        {
                            history_add_record(player_info_arr + i);
                        }

                        player_info_arr_len = 0;
                    }
                }

                goto CONNECTED_FINISH;
            }
        }
        CONNECTED_FINISH:
    }

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: Username at end: \"%s\"\n", user_name);

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error: ", collection_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}
