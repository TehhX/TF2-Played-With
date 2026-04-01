#include "collection.h"

#include "common.h"
#include "history.h"
#include "player_info.h"
#include "steamid_manip.h"
#include "time_manip.h"

#include "cider.h"

#include "stdio.h"
#include "string.h"
#include "inttypes.h"
#include "stdatomic.h"
#include "stdbool.h"

#ifdef __linux__
    #include "unistd.h"

    #define tf2pw_sleep sleep
#elif defined(_WIN32) || defined(_WIN64)
    #include "windows.h"

    // MAJOR_TODO: Test this
    #define tf2pw_sleep(SECONDS) Sleep((SECONDS) * 1000)
#else
    #error "Unknown OS"
#endif

// MAJOR_TODO: If new people keep joining a match this will overflow. Switch to heap allocation for pinfo_arr->arr
// How many players can be in a match (including user)
#define MAX_PLAYERS 256

// MAJOR_TODO: Read line dynamically into heap instead of using stack char arr
// Size of the line buffer in bytes. Should be a couple bytes larger than the largest expected line length. Keep in mind that values via pow(2, (int) exp) eg. 512 make it seem up to 8 times more professional
#define LINE_BUFB 256

// Whether archive or live
#define COLLECTION_LIVE    ((bool) false)
#define COLLECTION_ARCHIVE ((bool) true)

struct player_info_arr
{
    struct player_info arr[MAX_PLAYERS];
    int len;
};

static void parse_log(FILE *file_stream, const bool caller, steam_name_stack user_name, struct player_info_arr *pinfo_arr)
{
    TF2_PLAYED_WITH_DEBUG_INSERT(size_t file_line_index = 1; int match_index = 1;)

    for (char line_buf[LINE_BUFB]; fgets(line_buf, LINE_BUFB, file_stream); TF2_PLAYED_WITH_DEBUG_INSERT(++file_line_index))
    {
        // All status lines containing a name-sid3e have 2 spaces after the octothorpe, title only has 1
        #define STATUS_PREFIX "#  "
        #define STATUS_PREFIX_SIZE (sizeof(STATUS_PREFIX) - 1)

        // Check for status output
        int user_name_len;
        if (!memcmp(line_buf, STATUS_PREFIX, STATUS_PREFIX_SIZE))
        {
            int line_i = STATUS_PREFIX_SIZE;

            // Get player name start index
            while (line_buf[line_i++] != '"');
            const int player_name_begin = line_i;

            // This is user, skip
            if (!strncmp(user_name, line_buf + player_name_begin, user_name_len))
            {
                goto STATUS_FINISH;
            }

            // Get index of last closing bracket and last occurrence of "BOT"
            int last_close_bracket_i = 0, last_bot_str_i = 0;
            while (line_buf[++line_i] != '\n')
            {
                if (line_buf[line_i] == ']')
                {
                    last_close_bracket_i = line_i;
                }
                else if (!strncmp(line_buf + line_i, "BOT", 3))
                {
                    last_bot_str_i = line_i;
                }
            }

            // This is a bot, skip. More information at /README-technical.md#L38
            if (last_bot_str_i >= last_close_bracket_i)
            {
                TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: (LI=%zu) Bot found, skipping.\n" ANSI_RESET, file_line_index);)
                goto STATUS_FINISH;
            }

            // Set line_i to index of start of SID3E
            for (line_i = last_close_bracket_i; line_buf[--line_i - 1] != ':'; );

            const uint32_t current_sid3e = sidm_parse_sid3e(line_buf + line_i - sizeof("[U:1:") + 1, Esteamid_type_sid3);
            if (current_sid3e >= SIDM_ERR_NONE_MAX)
            {
                TF2_PLAYED_WITH_DEBUG_CHOOSE
                (
                    fprintf(stderr, ANSI_RED "FATAL: (LI=%zu) Bad current_sid3e read from status output.\n" ANSI_RESET, file_line_index);
                    ,
                    fputs(ANSI_RED "MAJOR: Bad current_sid3e read from status output.\n" ANSI_RESET, stderr);
                )

                TF2_PLAYED_WITH_DEBUG_ABEX();
            }

            // Sentinel value if current SID3E is not found in arr
            #define CURRENT_NOT_FOUND ((int) MAX_PLAYERS)

            // BSEARCH_TODO
            int current_arr_index = CURRENT_NOT_FOUND;
            for (int i = 0; i < pinfo_arr->len; ++i)
            {
                if (pinfo_arr->arr[i].sid3e == current_sid3e)
                {
                    current_arr_index = i;
                    break;
                }
            }

            // Player not in status array, add them
            if (current_arr_index == CURRENT_NOT_FOUND)
            {
                current_arr_index = pinfo_arr->len++;

                // Get player name end index
                while (line_buf[--line_i] != '"');
                const int player_name_len = line_i - player_name_begin;

                // Set name and SID3E in pinfo_arr->arr
                memcpy(pinfo_arr->arr[current_arr_index].name, line_buf + player_name_begin, player_name_len);
                pinfo_arr->arr[current_arr_index].name[player_name_len] = '\0';
                pinfo_arr->arr[current_arr_index].sid3e                 = current_sid3e;

                // A million of these lines will be printed for an average log file, may or may not be commented out for any given commit
                // TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: (LI=%llu) Status line of new player #%3d in match #%2zu parsed: (\"%s\", %" PRIu32 ")\n" ANSI_RESET, file_line_index, current_arr_index + 1, match_index, pinfo_arr->arr[current_arr_index].name, pinfo_arr->arr[current_arr_index].sid3e);)

                if (caller == COLLECTION_LIVE)
                {
                    history_add_record(pinfo_arr->arr + pinfo_arr->len - 1);
                }
            }

            #undef CURRENT_NOT_FOUND
        }
        STATUS_FINISH:;

        // Check for connected message in the form `<NAME> connected`
        #define CONNECTED_SUFFIX "connected"
        #define CONNECTED_SUFFIX_SIZE ((int) sizeof(CONNECTED_SUFFIX) - 1)

        for (int i = 0; i < LINE_BUFB; ++i)
        {
            if (line_buf[i] == '\n')
            {
                // Line is not long enough to house connected statement, checking may cause a program crash and is pointless anyway
                if (i <= (int) CONNECTED_SUFFIX_SIZE)
                {
                    goto CONNECTED_FINISH;
                }

                if (!memcmp(line_buf + i - CONNECTED_SUFFIX_SIZE, CONNECTED_SUFFIX, CONNECTED_SUFFIX_SIZE))
                {
                    const int name_end = i - CONNECTED_SUFFIX_SIZE - 1;

                    // Is first occurrence of username
                    if (user_name[0] == '\0')
                    {
                        memcpy(user_name, line_buf, name_end);
                        user_name[name_end] = '\0';
                        user_name_len = name_end;

                        pinfo_arr->len = 0;

                        TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: (LI=%zu) First occurrence of username connected: \"%s\"\n" ANSI_RESET, file_line_index, user_name);)
                    }
                    // Is another occurrence of username
                    else if (!memcmp(line_buf, user_name, name_end))
                    {
                        TF2_PLAYED_WITH_DEBUG_INSERT(++match_index;)

                        TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: (LI=%zu) Another occurrence of username connected: \"%.*s\"\n" ANSI_RESET, file_line_index, name_end, line_buf);)

                        if (caller == COLLECTION_LIVE)
                        {
                            history_set_date(HISTORY_SET_DATE_TODAY);
                        }
                        else
                        {
                            for (int i = 0; i < pinfo_arr->len; ++i)
                            {
                                history_add_record(pinfo_arr->arr + i);
                            }
                        }

                        pinfo_arr->len = 0;
                    }
                }

                goto CONNECTED_FINISH;
            }
        }
        CONNECTED_FINISH:;
    }

    if (caller == COLLECTION_ARCHIVE)
    {
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Username at end: \"%s\"\n" ANSI_RESET, user_name);
    }
}

void *collection_read_live_routine(void *_params)
{
    struct collection_read_live_routine_params *params = _params;

    steam_name_stack user_name = { '\0' };
    struct player_info_arr live_pinfo_arr = { .len = 0 };

    while (params->continue_running)
    {
        parse_log(params->input_file, COLLECTION_LIVE, user_name, &live_pinfo_arr);
        clearerr(params->input_file);
        tf2pw_sleep(1);
    }

    return NULL;
}

void collection_read_archived(const char *collection_fullname)
{
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "Opening archived-log \"%s\".\n", collection_fullname);

    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to open archive-file \"%s\" for reading. Error: ", collection_fullname);
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    history_set_date(time_manip_ues2ued(cider_creation_date_file(collection_fullname)));

    parse_log(input_file_ptr, COLLECTION_ARCHIVE, (steam_name_stack){ '\0' }, &(struct player_info_arr){ .len = 0 });

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to close \"%s\". Error: ", collection_fullname);
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}
