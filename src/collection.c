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
#include "stdbool.h"

#ifdef __linux__
    #include "unistd.h"

    #define tf2pw_sleep sleep
#elif defined(_WIN32) || defined(_WIN64)
    #include "windows.h"

    #define tf2pw_sleep(SECONDS) Sleep((SECONDS) * 1000)
#endif

// How many players can be in a match (including user)
#define MAX_PLAYERS 512

// Size of the line buffer in bytes. Should be a couple bytes larger than the largest expected line length. Keep in mind that values via pow(2, (int) exp) eg. 512 make it seem up to 8 times more professional
#define LINE_BUFB 512

// Whether archive or live
#define COLLECTION_LIVE    ((bool) false)
#define COLLECTION_ARCHIVE ((bool) true)

struct player_info_arr
{
    struct player_info arr[MAX_PLAYERS];
    int len;
};

// Returns true if S1 matches S2. Will use length of S2
#define STRINGS_MATCH(S1, S2) (!memcmp(S1, S2, sizeof(S2) - 1))

// Returns true if line_buf matches STRING
#define LINE_MATCHES(STRING) STRINGS_MATCH(line_buf, STRING)

// @returns True for fail, false for success
static bool parse_log(FILE *file_stream, const bool caller, struct player_info_arr *pinfo_arr)
{
    TF2_PLAYED_WITH_DEBUG_INSERT(size_t file_line_index = 1;)

    for (char line_buf[LINE_BUFB]; fgets(line_buf, LINE_BUFB, file_stream); TF2_PLAYED_WITH_DEBUG_INSERT(++file_line_index))
    {
        // All status lines containing a name-sid3e have 2 spaces after the octothorpe, title only has 1
        #define STATUS_PREFIX "#  "

        // Check for status output
        if (LINE_MATCHES(STATUS_PREFIX))
        {
            int line_i = sizeof(STATUS_PREFIX) - 1;

            // Get player name start index
            while (line_buf[line_i++] != '"');
            const int player_name_begin = line_i;

            // Get index of last closing bracket and last occurrence of "BOT"
            int last_close_bracket_i = 0, last_bot_str_i = 0;
            while (line_buf[++line_i] != '\n')
            {
                if (line_buf[line_i] == ']')
                {
                    last_close_bracket_i = line_i;
                }
                else if (!memcmp(line_buf + line_i, "BOT", sizeof("BOT") - 1))
                {
                    last_bot_str_i = line_i;
                }
            }

            // This is a bot, skip
            if (last_bot_str_i >= last_close_bracket_i)
            {
                TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: (LI=%zu) Bot found, skipping.\n" ANSI_RESET, file_line_index);)
                goto STATUS_FINISH;
            }

            // Set line_i to index of start of SID3E
            for (line_i = last_close_bracket_i; line_buf[--line_i - 1] != ':'; );

            // Get SID3E
            const uint32_t current_sid3e = sidm_parse_sid3e(line_buf + line_i - sizeof("[U:1:") + 1, Esteamid_type_sid3);
            if (current_sid3e >= SIDM_ERR_NONE_MAX)
            {
                TF2_PLAYED_WITH_DEBUG_CHOOSE
                (
                    fprintf(stderr, ANSI_RED "(LI=%zu) Bad current_sid3e read from status output.\n" ANSI_RESET, file_line_index);

                    return true;
                    ,
                    fputs(ANSI_RED "Bad current_sid3e read from status output.\n" ANSI_RESET, stderr);

                    return true;
                )
            }

            // If equals user SID3E, skip
            if (current_sid3e == history_get_user_sid3e())
            {
                goto STATUS_FINISH;
            }

            // BSEARCH_TODO
            // Skip if found in array, else add new
            for (int i = 0; i < pinfo_arr->len; ++i)
            {
                // Player found in array, skip
                if (pinfo_arr->arr[i].sid3e == current_sid3e)
                {
                    goto STATUS_FINISH;
                }
            }

            // Player not in status array, add them
            ++pinfo_arr->len;

            // Get player name end index
            while (line_buf[--line_i] != '"');
            const int player_name_len = line_i - player_name_begin;

            // Set name and SID3E in pinfo_arr->arr
            memcpy(pinfo_arr->arr[pinfo_arr->len - 1].name, line_buf + player_name_begin, player_name_len);
            pinfo_arr->arr[pinfo_arr->len - 1].name[player_name_len] = '\0';
            pinfo_arr->arr[pinfo_arr->len - 1].sid3e                 = current_sid3e;

            // Add new player to records
            history_add_record(pinfo_arr->arr + pinfo_arr->len - 1);
        }
        STATUS_FINISH:;

        // Check for connected message in the form `<NAME> connected`
        {
            #define NEW_MATCH "Client reached server_spawn."

            // Detected NEW_MATCH
            if (LINE_MATCHES(NEW_MATCH))
            {
                TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Detected \"" NEW_MATCH "\", starting new match.\n" ANSI_RESET);

                if (caller == COLLECTION_LIVE)
                {
                    history_set_date(HISTORY_SET_DATE_TODAY);
                }

                pinfo_arr->len = 0;
            }
        }

        // Check for chat message
        {
            // Get name beginning
            #define PREFIX_DEAD "*DEAD*"
            #define PREFIX_TEAM "(TEAM)"
            #define MESSAGE_MID " :  "

            const char *current_name = line_buf, *message;

            if (LINE_MATCHES(PREFIX_DEAD))
            {
                current_name += sizeof(PREFIX_DEAD);

                if (STRINGS_MATCH(current_name - 1, PREFIX_TEAM))
                {
                    current_name += sizeof(PREFIX_TEAM) - 1;
                }
            }
            else if (LINE_MATCHES(PREFIX_TEAM))
            {
                current_name += sizeof(PREFIX_TEAM);
            }

            // Check who said it
            for (int i = 0; i < pinfo_arr->len; ++i)
            {
                const int name_len = (int) strlen(pinfo_arr->arr[i].name);

                // Found them, request to add
                if (!memcmp(pinfo_arr->arr[i].name, current_name, name_len))
                {
                    if (!memcmp(current_name + name_len, MESSAGE_MID, sizeof(MESSAGE_MID) - 1))
                    {
                        message = current_name + name_len + sizeof(MESSAGE_MID) - 1;

                        history_add_message(pinfo_arr->arr[i].sid3e, message);
                    }
                }
            }
        }
    }

    return false;
}

void *_collection_read_live_routine(struct collection_read_live_routine_params *params)
{
    struct player_info_arr live_pinfo_arr = { .len = 0 };
    history_set_date(HISTORY_SET_DATE_TODAY);

    params->running = true;

    while (params->running)
    {
        if (parse_log(params->input_file, COLLECTION_LIVE, &live_pinfo_arr))
        {
            params->running = false;

            return NULL;
        }

        clearerr(params->input_file);
        tf2pw_sleep(5);
    }

    params->running = false;

    return NULL;
}

void collection_read_archived(const char *collection_fullname)
{
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "Opening archived-log \"%s\".\n", collection_fullname);

    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, ANSI_RED "Failed to open archive-file \"%s\" for reading: ", collection_fullname);
        perror(NULL);
        RESET_STDERR_COL();

        return;
    }

    history_set_date(time_manip_ues2ued(cider_creation_date_file(collection_fullname)));

    if (parse_log(input_file_ptr, COLLECTION_ARCHIVE, &(struct player_info_arr){ .len = 0 }))
    {
        fprintf(stderr, ANSI_RED "Failed to parse file \"%s\": ", collection_fullname);
        perror(NULL);
        RESET_STDERR_COL();

        return;
    }

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\": ", collection_fullname);
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }
}
