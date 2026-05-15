#include "collection.h"

#include "common.h"
#include "history.h"
#include "player_info.h"
#include "steamid_manip.h"
#include "time_manip.h"
#include "file_io.h"

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

// Whether archive or live
#define COLLECTION_LIVE    false
#define COLLECTION_ARCHIVE true

struct parse_info
{
    struct player_info *player_info_arr;
    int len;
    bool treat_next_match_as_current; // Will negate next new match scan one time, mainly `retry` command related https://github.com/TehhX/TF2-Played-With/issues/10
};

// Returns true if S1 matches S2. Will use length of S2
#define STRINGS_MATCH(S1, S2) (!memcmp(S1, S2, sizeof(S2) - 1))

static bool scan_status(const char *line, const size_t line_len, struct parse_info *const parse_info)
{
    // All status lines containing a name-sid3e have 2 spaces after the octothorpe
    static const char status_prefix[] = "#  ";

    // If not status output, skip
    if ((line_len < sizeof(status_prefix) - 1) || !STRINGS_MATCH(line, status_prefix))
    {
        return false;
    }

    int line_i = sizeof(status_prefix) - 1;

    // Get player name start index
    while (line[line_i++] != '"');
    const int player_name_begin = line_i;

    // Get index of last closing bracket and last occurrence of "BOT"
    int last_close_bracket_i = 0, last_bot_str_i = 0;
    while (++line_i < (((int) line_len) - (int) sizeof("BOT") - 1))
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
        TF2_PLAYED_WITH_DEBUG_LOGS("Bot found, skipping.\n");
        return true;
    }

    // Set line_i to index of start of SID3E
    for (line_i = last_close_bracket_i; line[--line_i - 1] != ':'; );

    // Get SID3E
    const uint32_t current_sid3e = sidm_parse_sid3e(line + line_i - sizeof("[U:1:") + 1, Esteamid_type_sid3);
    if (current_sid3e >= SIDM_ERR_NONE_MAX)
    {
        fputs(ANSI_RED "Bad current_sid3e read from status output.\n" ANSI_RESET, stderr);
        return false;
    }

    // If equals user SID3E, skip
    if (current_sid3e == history_get_user_sid3e())
    {
        return true;
    }

    // BSEARCH_TODO
    // Skip if found in array, else add new
    for (int i = 0; i < parse_info->len; ++i)
    {
        // Player found in array, skip
        if (parse_info->player_info_arr[i].sid3e == current_sid3e)
        {
            return true;
        }
    }

    // Player not in status array, add them
    prealloc(parse_info->player_info_arr, ++parse_info->len);

    // Get player name end index
    while (line[--line_i] != '"');
    const int player_name_len = line_i - player_name_begin;

    parse_info->player_info_arr[parse_info->len - 1].name = malloc(sizeof(char) * (player_name_len + 1));
    memcpy(parse_info->player_info_arr[parse_info->len - 1].name, line + player_name_begin, player_name_len);
    parse_info->player_info_arr[parse_info->len - 1].name[player_name_len] = '\0';

    parse_info->player_info_arr[parse_info->len - 1].sid3e                 = current_sid3e;

    // Add new player to records
    history_add_record((parse_info->player_info_arr) + parse_info->len - 1);

    return true;
}

HYPER_MACRO void names_free(struct parse_info *const parse_info)
{
    for (int i = 0; i < parse_info->len; ++i)
    {
        free(parse_info->player_info_arr[i].name);
    }
}

static bool scan_new_match(const char *line, const size_t line_len, struct parse_info *const parse_info, const bool collection_type)
{
    static const char new_match_line[] = "Client reached server_spawn.";

    if (line_len < sizeof(new_match_line) - 1)
    {
        return false;
    }
    else if (!STRINGS_MATCH(line, new_match_line))
    {
        return false;
    }

    // Clear `treat_next_match_as_current` and skip if `treat_next_match_as_current` set
    if (parse_info->treat_next_match_as_current)
    {
        TF2_PLAYED_WITH_DEBUG_LOGS("Resetting treat_next_match_as_current and skipping new match.\n");

        parse_info->treat_next_match_as_current = false;
    }
    else
    {
        TF2_PLAYED_WITH_DEBUG_LOGS("Starting new match.\n");

        if (collection_type == COLLECTION_LIVE)
        {
            history_set_date(HISTORY_SET_DATE_TODAY);
        }

        names_free(parse_info);

        parse_info->len = 0;
    }

    return true;
}

static bool scan_message(const char *line, const size_t line_len, const struct parse_info *const parse_info)
{
    // Get name beginning
    static const char
        prefix_dead[] = "*DEAD*",
        prefix_team[] = "(TEAM)",
        message_mid[] = " :  "
    ;

    if (line_len < sizeof(message_mid) + 2)
    {
        return false;
    }
    else if (STRINGS_MATCH(line, prefix_dead))
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
        const int name_len = (int) strlen(parse_info->player_info_arr[i].name);

        // Check for a name match after preamble
        if (!strncmp(parse_info->player_info_arr[i].name, line, name_len))
        {
            // Check that MESSAGE_MID follows name
            if (!strncmp(line + name_len, message_mid, sizeof(message_mid) - 1))
            {
                history_add_message(parse_info->player_info_arr[i].sid3e, line + name_len + sizeof(message_mid) - 1);
                return true;
            }
        }
    }

    return false;
}

static bool scan_retry(const char *line, const size_t line_len, struct parse_info *const parse_info)
{
    static const char retry_line[] = "Commencing connection retry to ";

    if (line_len < sizeof(retry_line))
    {
        return false;
    }
    else if (STRINGS_MATCH(line, retry_line))
    {
        TF2_PLAYED_WITH_DEBUG_LOGS("Found retry statement, setting treat_next_match_as_current.\n");

        parse_info->treat_next_match_as_current = true;

        return true;
    }
    else
    {
        return false;
    }
}

static void parse_log(FILE *file_stream, const bool collection_type, struct parse_info *parse_info)
{
    char *line_buf = NULL;
    size_t line_buf_len;

    while (!feof(file_stream))
    {
        line_buf_len = file_io_buffered_input(file_stream, &line_buf);

        // Check from most common output type to least, stopping once one is found
             if (scan_status   (line_buf, line_buf_len, parse_info                 )) { ; }
        else if (scan_message  (line_buf, line_buf_len, parse_info                 )) { ; }
        else if (scan_new_match(line_buf, line_buf_len, parse_info, collection_type)) { ; }
        else if (scan_retry    (line_buf, line_buf_len, parse_info                 )) { ; }
    }

    free(line_buf);
    names_free(parse_info);
    free(parse_info->player_info_arr);
}

void *_collection_read_live_routine(struct collection_read_live_routine_params *params)
{
    history_set_date(HISTORY_SET_DATE_TODAY);

    params->running = true;

    for (struct parse_info live_parse_info = { .len = 0, .treat_next_match_as_current = false, .player_info_arr = NULL }; params->running; )
    {
        parse_log(params->input_file, COLLECTION_LIVE, &live_parse_info);

        // Will be EOF at this point, must be cleared
        clearerr(params->input_file);

        tf2pw_sleep(5);
    }

    params->running = false;

    return NULL;
}

void collection_read_archived(const char *collection_fullname)
{
    TF2_PLAYED_WITH_DEBUG_LOGF("Opening archived log \"%s\".\n", collection_fullname);

    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, ANSI_RED "Failed to open archive-file \"%s\" for reading: ", collection_fullname);
        perror(NULL);
        ANSI_RESET_STDERR();

        return;
    }

    history_set_date(time_manip_ues2ued(cider_creation_date_file(collection_fullname)));

    parse_log(input_file_ptr, COLLECTION_ARCHIVE, &(struct parse_info){ .len = 0, .treat_next_match_as_current = false, .player_info_arr = NULL });

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\": ", collection_fullname);
        perror(NULL);
        ANSI_RESET_STDERR();
        return;
    }
}
