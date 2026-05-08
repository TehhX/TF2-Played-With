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

struct parse_info
{
    struct player_info arr[MAX_PLAYERS];
    int len;
    bool treat_next_match_as_current; // Will negate next new match scan one time, mainly `retry` command related https://github.com/TehhX/TF2-Played-With/issues/10
};

// Returns true if S1 matches S2. Will use length of S2
#define STRINGS_MATCH(S1, S2) (!memcmp(S1, S2, sizeof(S2) - 1))

static void scan_status(const char *line, struct parse_info *const parse_info)
{
    // All status lines containing a name-sid3e have 2 spaces after the octothorpe
    static const char status_prefix[] = "#  ";

    // If not status output, skip
    if (!STRINGS_MATCH(line, status_prefix))
    {
        return;
    }

    int line_i = sizeof(status_prefix) - 1;

    // Get player name start index
    while (line[line_i++] != '"');
    const int player_name_begin = line_i;

    // Get index of last closing bracket and last occurrence of "BOT"
    int last_close_bracket_i = 0, last_bot_str_i = 0;
    while (line[++line_i] != '\n')
    {
        if (line[line_i] == ']')
        {
            last_close_bracket_i = line_i;
        }
        else if (!memcmp(line + line_i, "BOT", sizeof("BOT") - 1))
        {
            last_bot_str_i = line_i;
        }
    }

    // This is a bot, skip
    if (last_bot_str_i >= last_close_bracket_i)
    {
        TF2_PLAYED_WITH_DEBUG_INSERT(printf(ANSI_LOG "LOG: Bot found, skipping.\n" ANSI_RESET);)
        return;
    }

    // Set line_i to index of start of SID3E
    for (line_i = last_close_bracket_i; line[--line_i - 1] != ':'; );

    // Get SID3E
    const uint32_t current_sid3e = sidm_parse_sid3e(line + line_i - sizeof("[U:1:") + 1, Esteamid_type_sid3);
    if (current_sid3e >= SIDM_ERR_NONE_MAX)
    {
        fputs(ANSI_RED "Bad current_sid3e read from status output.\n" ANSI_RESET, stderr);
        return;
    }

    // If equals user SID3E, skip
    if (current_sid3e == history_get_user_sid3e())
    {
        return;
    }

    // BSEARCH_TODO
    // Skip if found in array, else add new
    for (int i = 0; i < parse_info->len; ++i)
    {
        // Player found in array, skip
        if (parse_info->arr[i].sid3e == current_sid3e)
        {
            return;
        }
    }

    // Player not in status array, add them
    ++parse_info->len;

    // Get player name end index
    while (line[--line_i] != '"');
    const int player_name_len = line_i - player_name_begin;

    // Set name and SID3E in parse_info->arr
    memcpy(parse_info->arr[parse_info->len - 1].name, line + player_name_begin, player_name_len);

    parse_info->arr[parse_info->len - 1].name[player_name_len] = '\0';
    parse_info->arr[parse_info->len - 1].sid3e                 = current_sid3e;

    // Add new player to records
    history_add_record(parse_info->arr + parse_info->len - 1);
}

static void scan_new_match(const char *line, struct parse_info *const parse_info, const bool caller)
{
    // Check for `Client reached server_spawn.`
    if (STRINGS_MATCH(line, "Client reached server_spawn."))
    {
        // Clear `treat_next_match_as_current` and skip if `treat_next_match_as_current` set
        if (parse_info->treat_next_match_as_current)
        {
            TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Resetting treat_next_match_as_current and skipping new match.\n" ANSI_RESET);

            parse_info->treat_next_match_as_current = false;
        }
        else
        {
            TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Starting new match.\n" ANSI_RESET);

            if (caller == COLLECTION_LIVE)
            {
                history_set_date(HISTORY_SET_DATE_TODAY);
            }

            parse_info->len = 0;
        }
    }
}

static void scan_message(const char *line, const struct parse_info *const parse_info)
{
    // Get name beginning
    static const char
        prefix_dead[] = "*DEAD*",
        prefix_team[] = "(TEAM)",
        message_mid[] = " :  "
    ;

    if (STRINGS_MATCH(line, prefix_dead))
    {
        line += sizeof(prefix_dead);

        if (STRINGS_MATCH(line - 1, prefix_team))
        {
            line += sizeof(prefix_team) - 1;
        }
    }
    else if (STRINGS_MATCH(line, prefix_team))
    {
        line += sizeof(prefix_team);
    }

    for (int i = 0; i < parse_info->len; ++i)
    {
        const int name_len = (int) strlen(parse_info->arr[i].name);

        // Check for a name match after preamble
        if (!memcmp(parse_info->arr[i].name, line, name_len))
        {
            // Check that MESSAGE_MID follows name
            if (!memcmp(line + name_len, message_mid, sizeof(message_mid) - 1))
            {
                history_add_message(parse_info->arr[i].sid3e, line + name_len + sizeof(message_mid) - 1);
                return;
            }
        }
    }
}

static void scan_retry(const char *line, struct parse_info *const parse_info)
{
    if (STRINGS_MATCH(line, "Commencing connection retry to "))
    {
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Found retry statement, setting treat_next_match_as_current.\n");

        parse_info->treat_next_match_as_current = true;
    }
}

// @returns True for fail, false for success
static void parse_log(FILE *file_stream, const bool caller, struct parse_info *parse_info)
{
    for (char line_buf[LINE_BUFB]; fgets(line_buf, LINE_BUFB, file_stream); )
    {
        scan_status(line_buf, parse_info);

        scan_new_match(line_buf, parse_info, caller);

        scan_message(line_buf, parse_info);

        scan_retry(line_buf, parse_info);
    }
}

void *_collection_read_live_routine(struct collection_read_live_routine_params *params)
{
    history_set_date(HISTORY_SET_DATE_TODAY);

    params->running = true;

    for (struct parse_info live_parse_info = { .len = 0, .treat_next_match_as_current = false }; params->running; )
    {
        parse_log(params->input_file, COLLECTION_LIVE, &live_parse_info);

        clearerr(params->input_file);
        tf2pw_sleep(5);
    }

    params->running = false;

    return NULL;
}

void collection_read_archived(const char *collection_fullname)
{
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "Opening archived log \"%s\".\n", collection_fullname);

    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, ANSI_RED "Failed to open archive-file \"%s\" for reading: ", collection_fullname);
        perror(NULL);
        RESET_STDERR_COL();

        return;
    }

    history_set_date(time_manip_ues2ued(cider_creation_date_file(collection_fullname)));

    parse_log(input_file_ptr, COLLECTION_ARCHIVE, &(struct parse_info){ .len = 0, .treat_next_match_as_current = false });

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\": ", collection_fullname);
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }
}
