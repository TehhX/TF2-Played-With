#include "history.h"

#include "common.h"
#include "time_manip.h"
#include "steamid_manip.h"
#include "user_input.h"
#include "save_formats/main.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

static struct save_format_data history_main_data = { .data_v0 = NULL };

void history_set_user_sid3e(const uint32_t new_user_sid3e)
{
    history_main_data.data_v0->user_sid3e = new_user_sid3e;

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set user_sid3e to %" PRIu32 ".\n" ANSI_RESET, new_user_sid3e);
}

uint32_t history_get_user_sid3e()
{
    return history_main_data.data_v0->user_sid3e;
}

// Frees memory associated with history, sets `player_records_len` to 0
void history_free()
{
    if (!history_main_data.data_v0)
    {
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Attempted history_free(...) while data not allocated, ignoring.\n" ANSI_RESET);
        return;
    }
    else if (!history_main_data.data_v0->player_records_len)
    {
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Attempted history_free(...) while player_records_len == 0, ignoring.\n" ANSI_RESET);
        return;
    }

    for (uint32_t player_i = 0; player_i < history_main_data.data_v0->player_records_len; ++player_i)
    {
        for (uint32_t date_i = 0; date_i < history_main_data.data_v0->player_records[player_i].date_records_len; ++date_i)
        {
            if (history_main_data.data_v0->player_records[player_i].date_records[date_i].name_len)
            {
                free(history_main_data.data_v0->player_records[player_i].date_records[date_i].name);
            }

            if (history_main_data.data_v0->player_records[player_i].record_messages && history_main_data.data_v0->player_records[player_i].date_records[date_i].messages)
            {
                for (size_t msg_i = 0; msg_i < history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len; ++msg_i)
                {
                    free(history_main_data.data_v0->player_records[player_i].date_records[date_i].messages[msg_i]);
                }

                free(history_main_data.data_v0->player_records[player_i].date_records[date_i].messages);
            }
        }

        free(history_main_data.data_v0->player_records[player_i].date_records);
        free(history_main_data.data_v0->player_records[player_i].notes);
    }

    free(history_main_data.data_v0->player_records);
    free(history_main_data.data_v0->tf2_filepath);
    free(history_main_data.data_v0->tf2_live_log_fullname);

    free(history_main_data.data_v0);
    history_main_data.data_v0 = NULL;
}

// @returns 1 for fail, 0 for success
HYPER_MACRO bool history_wizard()
{
    if (!user_input_confirm("No history file found. Start setup? (Y/N): ", NULL))
    {
        fputs(ANSI_RED "Either modify another history file or accept setup of a new file. Exiting.\n" ANSI_RESET, stderr);
        exit(EXIT_FAILURE);
    }

    history_main_data.save_version = SAVE_FORMAT_VERSION_LATEST;
    history_main_data.data_v0 = malloc(sizeof(*history_main_data.data_v0));

    char *user_input = NULL;

    while (user_input_getline(&user_input, "Enter path to TF2 eg. (..." CIDER_PATH_DELIM_S "Team Fortress 2" CIDER_PATH_DELIM_S "): ", NULL) == NULL || user_input[0] == '\0');

    char *proposed_tf2_filepath = string_deep_copy(user_input);
    if ((proposed_tf2_filepath = history_set_tf2_filepath(proposed_tf2_filepath)) == NULL)
    {
        free(proposed_tf2_filepath);
        free(user_input);
        return true;
    }

    #define TF2PW_CFG_SEMINAME "tf" CIDER_PATH_DELIM_S "cfg" CIDER_PATH_DELIM_S

    if (user_input_confirm("Append con_logfile to autoexec? (Y/N): ", NULL))
    {
        char *autoexec_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v0->tf2_filepath), TF2PW_AUTOEXEC_SEMINAME);
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set autoexec_fullname to \"%s\".\n" ANSI_RESET, autoexec_fullname);

        FILE *autoexec_handle = fopen(autoexec_fullname, "a");
        if (!autoexec_handle)
        {
            fprintf(stderr, ANSI_RED "MAJOR: Failed to open autoexec file \"%s\" for appending: ", autoexec_fullname);
            perror(NULL);
            RESET_STDERR_COL();
            free(autoexec_fullname);

            return true;
        }

        fprintf(autoexec_handle, "// BEGIN Generated automatically by TF2PW, don't edit.\ncon_logfile " TF2PW_LOG_FILENAME "\n// END   Generated automatically by TF2PW, don't edit.\n");

        if (fclose(autoexec_handle))
        {
            fprintf(stderr, ANSI_RED "Failed to close autoexec file \"%s\": ", autoexec_fullname);
            perror(NULL);
            RESET_STDERR_COL();
            free(autoexec_fullname);

            return true;
        }

        free(autoexec_fullname);
    }

    if (user_input_confirm("Replace W bind with forward and status? (Y/N): ", NULL))
    {
        #define TF2PW_CONFIG_SEMINAME TF2PW_CFG_SEMINAME "config.cfg"
        #define TF2PW_TEMP_SEMINAME TF2PW_CFG_SEMINAME "tf2pw.cfg.tmp"

        char *config_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v0->tf2_filepath), TF2PW_CONFIG_SEMINAME);

        FILE *config_handle = fopen(config_fullname, "r");
        if (!config_handle)
        {
            fprintf(stderr, ANSI_RED "Failed to open config file \"%s\" for appending: ", config_fullname);
            perror(NULL);
            RESET_STDERR_COL();
            free(config_fullname);

            return true;
        }

        // Search for `bind "w" "+forward"`. If exists, replace with `bind "w" "+forward ; status"`
        const char
            config_to_replace [] = "bind \"w\" \"+forward\"",
            config_replacement[] = "bind \"w\" \"+forward ; status\""
        ;

        char *temporary_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v0->tf2_filepath), TF2PW_TEMP_SEMINAME);
        FILE *file_output = fopen(temporary_fullname, "w");
        if (!file_output)
        {
            fprintf(stderr, ANSI_RED "Failed to open temporary file \"%s\" for writing.\n" ANSI_RESET, temporary_fullname);
            free(config_fullname);

            return true;
        }

        char *temp_buf = NULL;
        int config_input_char;

        do
        {
            size_t buf_len = 0;

            bool
                found = true,
                in_offender = true
            ;

            while ((config_input_char = fgetc(config_handle)) != '\n' && config_input_char != EOF)
            {
                // Allocate space for, assign config_input_char onto temp_buf
                temp_buf = realloc(temp_buf, sizeof(char) * (++buf_len));
                temp_buf[buf_len - 1] = (char) config_input_char;

                found =
                    (found) && // Has matched up to this point
                    (in_offender = in_offender && (config_to_replace[buf_len - 1] != '\0')) && // Haven't gone past offender null terminator either before or just now
                    (config_to_replace[buf_len - 1] == config_input_char) // Matches on this character as well
                ;
            }

            if (buf_len && found && (config_to_replace[buf_len] == '\0'))
            {
                fprintf(file_output, "%s\n", config_replacement);
            }
            else if (buf_len)
            {
                fwrite(temp_buf, sizeof(char), buf_len, file_output);
                fputc('\n', file_output);
            }
        }
        while (config_input_char != EOF);

        free(temp_buf);

        if (fclose(file_output))
        {
            fprintf(stderr, ANSI_RED "Failed to close write file \"%s\": ", temporary_fullname);
            perror(NULL);
            RESET_STDERR_COL();

            free(config_fullname);
            free(temporary_fullname);

            return true;
        }

        if (fclose(config_handle))
        {
            fprintf(stderr, ANSI_RED "MAJOR: Failed to close config file \"%s\": ", config_fullname);
            perror(NULL);
            RESET_STDERR_COL();

            free(config_fullname);
            free(temporary_fullname);

            return true;
        }

        // Overwrite original file with temporary file to complete the process
        if (remove(config_fullname) || rename(temporary_fullname, config_fullname))
        {
            perror(ANSI_RED "Failed to move temporary config file contents to real config location, attempt manual move" ANSI_RESET);

            return true;
        }

        free(temporary_fullname);
        free(config_fullname);
    }

    history_main_data.data_v0->tf2_live_log_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v0->tf2_filepath), TF2PW_LOG_SEMINAME);
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set tf2_live_log_fullname to \"%s\".\n" ANSI_RESET, history_main_data.data_v0->tf2_live_log_fullname);

    while (true)
    {
        while (user_input_getline(&user_input, "Enter your STEAMID as one of [STEAMID3|STEAMID3E|STEAMID64]: ", NULL) == NULL || user_input[0] == '\0');

        const uint32_t new_user_sid3e = sidm_parse_sid3e(user_input, Esteamid_type_unknown);
        if (new_user_sid3e == SIDM_ERR_NAME || new_user_sid3e == SIDM_ERR_MISC)
        {
            fprintf(stderr, ANSI_RED "Bad ID value. Try again.\n" ANSI_RESET);
        }
        else if (new_user_sid3e == SIDM_ERR_RNGE)
        {
            fprintf(stderr, ANSI_RED "ID value too large. Try again.\n" ANSI_RESET);
        }
        else
        {
            TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set user-sid3e to %" PRIu32 ".\n" ANSI_RESET, new_user_sid3e);
            history_main_data.data_v0->user_sid3e = new_user_sid3e;
            break;
        }
    }

    history_main_data.data_v0->default_record_messages = user_input_confirm("Record chat messages by default (Y/N): ", NULL);

    free(user_input);

    return false;
}

static const char *_history_get_default_fullname_pre();

// @brief Get default fullname of history file at `<Data filepath>/tf2pw.sav`
static const char *(*history_get_default_fullname)() = _history_get_default_fullname_pre;

// @warning Don't use this unless inside one of the `_history_get_default_fullname_(pre|post)...` functions
static char *_default_history_fullname = NULL;

// @warning Don't call this directly, use `history_get_default_fullname()` instead
static const char *_history_get_default_fullname_post()
{
    return _default_history_fullname;
}

// @warning Don't call this directly, use `history_get_default_fullname()` instead
static const char *_history_get_default_fullname_pre()
{
    TF2_PLAYED_WITH_DEBUG_ABORT_IF(_default_history_fullname != NULL);

    history_get_default_fullname = _history_get_default_fullname_post;

    return (_default_history_fullname = cider_construct_fullname(cider_data_filepath(), "tf2pw.sav"));
}

void history_load(const char *const passed_history_fullname)
{
    const char *const history_fullname = (passed_history_fullname ? passed_history_fullname : history_get_default_fullname());

    FILE *const input_file_ptr = fopen(history_fullname, "r");
    if (input_file_ptr == NULL)
    {
        // No file, request to use default values
        if (errno == ENOENT)
        {
            errno = 0;

            // Exit prematurely if wizard failed
            if (history_wizard())
            {
                exit(EXIT_FAILURE);
            }

            history_free();
            history_save(history_fullname);
        }
        else
        {
            fprintf(stderr, ANSI_RED "Failed to open \"%s\" for reading. Error: ", history_fullname);
            perror(NULL);
            RESET_STDERR_COL();
        }

        return;
    }

    history_free();

    char header_buf[sizeof(HEADER) - 1];
    fread(header_buf, 1, sizeof(HEADER) - 1, input_file_ptr);
    if (strncmp(header_buf, HEADER, sizeof(HEADER) - 1))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Requested file \"%s\" is not a valid tf2pw history file.\n" ANSI_RESET, history_fullname);

        goto CLOSE_HISTORY_FILE;
    }

    fread_one(history_main_data.save_version);
    switch (history_main_data.save_version)
    {
        break;  default:
        {
            fprintf(stderr, ANSI_RED "Version of history file \"%s\" is not supported by this version of TF2PW, get a newer version at " TF2PW_HOMEPAGE_URL ".\n" ANSI_RESET, history_fullname);

            // IMMED_TODO: Replace with load and save returning booleans
            exit(EXIT_FAILURE);
        }
        break;   case 0:
        {
            struct save_format_0 *const new_data = save_format_0_load(history_main_data.data_v0, input_file_ptr);
            if (new_data)
            {
                history_main_data.data_v0 = new_data;
            }
            else
            {
                fprintf(stderr, ANSI_RED "Failed to load file \"%s\".\n", history_fullname);
            }
        }
    }

    if (EOF != fgetc(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "EOF not reached for file \"%s\". History file not valid, exiting.\n" ANSI_RESET, history_fullname);
        exit(EXIT_FAILURE);
    }

    CLOSE_HISTORY_FILE:;
    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        RESET_STDERR_COL();

        return;
    }
}

void history_save(const char *const passed_history_fullname)
{
    const char *const history_fullname = (passed_history_fullname ? passed_history_fullname : history_get_default_fullname());

    FILE *const output_file_ptr = fopen(history_fullname, "w");
    if (!output_file_ptr)
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to open \"%s\" for writing. Error: ", history_fullname);
        perror(NULL);
        RESET_STDERR_COL();

        return;
    }

    fwrite(HEADER, sizeof(char), sizeof(HEADER) - 1, output_file_ptr);

    save_format_0_save(history_main_data.data_v0, output_file_ptr);

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }
}

char *history_set_tf2_filepath(char *new_tf2_filepath)
{
    // Length of tf2_filepath without trailing slash and/or null-terminator
    size_t new_tf2_filepath_len = strlen(new_tf2_filepath);

    const bool had_trailing_delim = (new_tf2_filepath[new_tf2_filepath_len - 1] == CIDER_PATH_DELIM_C);

    // Append trailing slash if doesn't exist
    if (had_trailing_delim)
    {
        --new_tf2_filepath_len;
    }

    if (new_tf2_filepath_len > UINT8_MAX)
    {
        fprintf(stderr, ANSI_RED "New TF2 filepath length is too long, is %zu, should be at most %zu.\n" ANSI_RESET, new_tf2_filepath_len, (size_t) UINT8_MAX);
        return NULL;
    }

    if (!had_trailing_delim)
    {
        prealloc(new_tf2_filepath, sizeof(char), new_tf2_filepath_len + 2);
        new_tf2_filepath[new_tf2_filepath_len] = CIDER_PATH_DELIM_C;
        new_tf2_filepath[new_tf2_filepath_len + 1] = '\0';
    }

    free(history_main_data.data_v0->tf2_filepath);
    history_main_data.data_v0->tf2_filepath = new_tf2_filepath;

    history_main_data.data_v0->tf2_filepath_len = (uint8_t) new_tf2_filepath_len;

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Setting tf2_filepath to \"%s\".\n" ANSI_RESET, history_main_data.data_v0->tf2_filepath);

    return history_main_data.data_v0->tf2_filepath;
}

const char *history_get_live_log_fullname()
{
    return (const char *) history_main_data.data_v0->tf2_live_log_fullname;
}

void history_set_date(const uint16_t new_date)
{
    if (new_date == HISTORY_SET_DATE_TODAY)
    {
        history_main_data.data_v0->current_date = time_manip_current_ued();
    }
    else
    {
        history_main_data.data_v0->current_date = new_date;
    }

    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        fputs(ANSI_LOG "LOG: Set current date to ", stdout);
        time_manip_print_ued(history_main_data.data_v0->current_date);
        putchar('\n');
        SET_COLOR(stdout, ANSI_RESET);
    )
}

HYPER_MACRO void history_add_date_record(const uint32_t player_records_i, const steam_name_stack name)
{
    const uint_fast32_t date_records_i = history_main_data.data_v0->player_records[player_records_i].date_records_len;
    #define current_date_record history_main_data.data_v0->player_records[player_records_i].date_records[date_records_i]

    prealloc(history_main_data.data_v0->player_records[player_records_i].date_records, sizeof(*history_main_data.data_v0->player_records[player_records_i].date_records), ++history_main_data.data_v0->player_records[player_records_i].date_records_len);

    current_date_record.date = history_main_data.data_v0->current_date;
    current_date_record.messages_len = 0;
    current_date_record.messages = NULL;
    current_date_record.encounter_count = 0;

    // This player record has date records aka. is not new
    if (date_records_i)
    {
        // Search backwards for same name
        for (uint_fast32_t date_name_search_i = date_records_i - 1; date_records_i != UINT_FAST32_MAX; --date_name_search_i)
        {
            // Previous date record is real
            if (history_main_data.data_v0->player_records[player_records_i].date_records[date_name_search_i].name_len)
            {
                // If are the same, point this name to same place as last name
                if (!strcmp(history_main_data.data_v0->player_records[player_records_i].date_records[date_name_search_i].name, name))
                {
                    current_date_record.name = history_main_data.data_v0->player_records[player_records_i].date_records[date_name_search_i].name;
                    current_date_record.name_len = 0;

                    return;
                }
                // If not, create new name for this record
                else
                {
                    goto ALLOCATE_NAME;
                }
            }
        }
    }

    ALLOCATE_NAME:;
    const uint_fast8_t current_name_len = current_date_record.name_len = (uint8_t) strlen(name);
    current_date_record.name = malloc(sizeof(char) * current_name_len + 1);
    memcpy(current_date_record.name, name, current_name_len + 1);

    #undef current_date_record
}

// Sentinel value for if player index doesn't exist
#define PLAYER_INDEX_ENOENT UINT_FAST32_MAX

// BSEARCH_TODO
// Returns index of `requested_sid3e`
HYPER_MACRO uint_fast32_t get_player_index(const uint32_t requested_sid3e)
{
    for (uint_fast32_t player_records_i = 0; player_records_i < history_main_data.data_v0->player_records_len; ++player_records_i)
    {
        if (history_main_data.data_v0->player_records[player_records_i].sid3e == requested_sid3e)
        {
            return player_records_i;
        }
    }

    // Couldn't find
    return PLAYER_INDEX_ENOENT;
}

void history_add_record(const struct player_info *const pinfo)
{
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Record add requested for (%s, %" PRIu32 "). Requested", pinfo->name, pinfo->sid3e);

    const uint_fast32_t player_index = get_player_index(pinfo->sid3e);
    if (player_index != PLAYER_INDEX_ENOENT)
    {
        // Found requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is in records");
        for (uint_fast32_t date_records_i = 0; date_records_i < history_main_data.data_v0->player_records[player_index].date_records_len; ++date_records_i)
        {
            if (history_main_data.data_v0->player_records[player_index].date_records[date_records_i].date != history_main_data.data_v0->current_date)
            {
                continue;
            }

            TF2_PLAYED_WITH_DEBUG_LOGF(" on current_date %" PRIu16 ". Incrementing encounter count.\n" ANSI_RESET, history_main_data.data_v0->current_date);

            ++history_main_data.data_v0->player_records[player_index].date_records[date_records_i].encounter_count;

            return;
        }

        // No record found for current_date
        TF2_PLAYED_WITH_DEBUG_LOGF(", but not on current_date %" PRIu16 ". Adding new date record.\n" ANSI_RESET, history_main_data.data_v0->current_date);
        history_add_date_record(player_index, pinfo->name);

        return;
    }
    else
    {
        // Couldn't find requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is not in records. Adding new player and date records.\n" ANSI_RESET);

        prealloc(history_main_data.data_v0->player_records, sizeof(*history_main_data.data_v0->player_records), ++history_main_data.data_v0->player_records_len);
        history_main_data.data_v0->player_records[history_main_data.data_v0->player_records_len - 1].sid3e = pinfo->sid3e;
        history_main_data.data_v0->player_records[history_main_data.data_v0->player_records_len - 1].record_messages = history_main_data.data_v0->default_record_messages;
        history_main_data.data_v0->player_records[history_main_data.data_v0->player_records_len - 1].notes = NULL;
        history_main_data.data_v0->player_records[history_main_data.data_v0->player_records_len - 1].date_records_len = 0;
        history_main_data.data_v0->player_records[history_main_data.data_v0->player_records_len - 1].date_records = NULL;

        history_add_date_record(history_main_data.data_v0->player_records_len - 1, pinfo->name);
    }
}

void history_print_record(const uint32_t requested_sid3e)
{
    const uint_fast32_t player_index = get_player_index(requested_sid3e);
    if (player_index != PLAYER_INDEX_ENOENT)
    {
        printf("Records for requested player [U:1:%" PRIu32 "]:\n", requested_sid3e);

        if (history_main_data.data_v0->player_records[player_index].notes)
        {
            printf(LTAB "Notes:\n" LTAB LTAB "%s", history_main_data.data_v0->player_records[player_index].notes);
        }

        for (uint_fast32_t date_index = 0; date_index < history_main_data.data_v0->player_records[player_index].date_records_len; ++date_index)
        {
            printf(LTAB);
            time_manip_print_ued(history_main_data.data_v0->player_records[player_index].date_records[date_index].date);
            printf(":\n");

            printf(LTAB LTAB "Times encountered: %" PRIu8 "\n", history_main_data.data_v0->player_records[player_index].date_records[date_index].encounter_count + 1);
            printf(LTAB LTAB "Name: \"%s\"\n", history_main_data.data_v0->player_records[player_index].date_records[date_index].name);

            if (history_main_data.data_v0->player_records[player_index].record_messages && history_main_data.data_v0->player_records[player_index].date_records[date_index].messages)
            {
                printf(LTAB LTAB "Messages:\n");
                for (size_t msg_i = 0; msg_i < history_main_data.data_v0->player_records[player_index].date_records[date_index].messages_len; ++msg_i)
                {
                    printf(LTAB LTAB LTAB "%s\n", history_main_data.data_v0->player_records[player_index].date_records[date_index].messages[msg_i]);
                }
            }
        }
    }
    else
    {
        printf(ANSI_RED "Requested player SID3E(%" PRIu32 ") not found.\n" ANSI_RESET, requested_sid3e);
    }
}

void history_print_records(const char *const name)
{
    bool record_found = false;
    for (uint_fast32_t player_i = 0; player_i < history_main_data.data_v0->player_records_len; ++player_i)
    {
        for (uint_fast32_t date_i = 0; date_i < history_main_data.data_v0->player_records[player_i].date_records_len; ++date_i)
        {
            // If name is not pointer to previous copycat name, and matches requested name, print record of that player, continue to next player_i
            if (history_main_data.data_v0->player_records[player_i].date_records[date_i].name_len && !strcmp(history_main_data.data_v0->player_records[player_i].date_records[date_i].name, name))
            {
                record_found = true;
                history_print_record(history_main_data.data_v0->player_records[player_i].sid3e);
                continue;
            }
        }
    }

    if (!record_found)
    {
        fprintf(stderr, ANSI_RED "No records found with the name \"%s\".\n" ANSI_RESET, name);
    }
}

void history_edit_notes(uint32_t requested_sid3e)
{
    const uint_fast32_t player_index = get_player_index(requested_sid3e);
    if (player_index == PLAYER_INDEX_ENOENT)
    {
        fprintf(stderr, ANSI_RED "Requested player SID3E(%" PRIu32 ") not found.\n" ANSI_RESET, requested_sid3e);
        return;
    }

    char *temporary_edit_file = cider_construct_fullname(cider_temp_filepath(), "tf2pw_note_editor.txt");

    FILE *write = fopen(temporary_edit_file, "w");
    if (!write)
    {
        fprintf(stderr, ANSI_RED "Failed to open temp note-editing file: ");
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }

    if (history_main_data.data_v0->player_records[player_index].notes)
    {
        fputs(history_main_data.data_v0->player_records[player_index].notes, write);
        fputc('\n', write);
    }

    if (fclose(write))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp note-editing file: ");
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }

    // MAJOR_TODO: Test on windows
    const char *const editor = getenv("EDITOR");

    char cmd_buff[128];
    sprintf(cmd_buff, "%s %s", (editor ? editor : "vi"), temporary_edit_file);
    system(cmd_buff);

    FILE *read = fopen(temporary_edit_file, "r");
    if (!read)
    {
        fprintf(stderr, ANSI_RED "Failed to open temp note-editing file: ");
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }

    // BUFF_TODO
    int input;
    size_t notes_len = 0;
    while ((input = fgetc(read)) != EOF)
    {
        prealloc(history_main_data.data_v0->player_records[player_index].notes, sizeof(char), (notes_len + 1));
        history_main_data.data_v0->player_records[player_index].notes[notes_len++] = (char) input;
    }

    if (notes_len > 0)
    {
        // TODO: Add newline if there is none at EOF
        history_main_data.data_v0->player_records[player_index].notes[notes_len - 1] = '\0';
    }
    // If user entered nothing/deleted all notes, free and set to NULL
    else
    {
        free(history_main_data.data_v0->player_records[player_index].notes);
        history_main_data.data_v0->player_records[player_index].notes = NULL;
    }

    if (fclose(read))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp note-editing file: ");
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }

    if (remove(temporary_edit_file))
    {
        fprintf(stderr, ANSI_RED "Failed to delete temp note-editing file: ");
        perror(NULL);
        RESET_STDERR_COL();
        return;
    }

    free(temporary_edit_file);
}

void history_add_message(const uint32_t requested_sid3e, const char *const message)
{
    // WARNING: Below assumes player has record and record on this date. Should always be true, however
    const uint_fast32_t player_i = get_player_index(requested_sid3e);
    if (history_main_data.data_v0->player_records[player_i].record_messages)
    {
        // BSEARCH_TODO
        for (uint_fast32_t date_i = 0; date_i < history_main_data.data_v0->player_records[player_i].date_records_len; ++date_i)
        {
            if (history_main_data.data_v0->player_records[player_i].date_records[date_i].date == history_main_data.data_v0->current_date)
            {
                int message_len = 0;
                for (; message[message_len] != '\n'; ++message_len);

                prealloc(history_main_data.data_v0->player_records[player_i].date_records[date_i].messages, sizeof(char *), ++history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len);
                history_main_data.data_v0->player_records[player_i].date_records[date_i].messages[history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len - 1] = malloc(sizeof(char) * (message_len + 1));
                history_main_data.data_v0->player_records[player_i].date_records[date_i].messages[history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len - 1][message_len] = '\0';
                memcpy(history_main_data.data_v0->player_records[player_i].date_records[date_i].messages[history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len - 1], message, message_len);

                TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Message add requested: (%" PRIu32 ", \"%s\").\n" ANSI_RESET, requested_sid3e, history_main_data.data_v0->player_records[player_i].date_records[date_i].messages[history_main_data.data_v0->player_records[player_i].date_records[date_i].messages_len - 1]);
                return;
            }
        }

        // Should have returned above. Getting here means there was no applicable record
        fprintf(stderr, "FATAL: No applicable record in history_add_message(...) for SID3E=%" PRIu32 ".\n", requested_sid3e);
        abort();
    }
}
