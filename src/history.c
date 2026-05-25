#include "history.h"

#include "common.h"
#include "time_manip.h"
#include "steamid_manip.h"
#include "user_input.h"
#include "save_formats/main.h"
#include "file_io.h"
#include "array_manip.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

static struct save_format_data history_main_data;

void history_set_user_sid3e(const uint32_t new_user_sid3e)
{
    history_main_data.data_v1.user_sid3e = new_user_sid3e;

    TF2_PLAYED_WITH_DEBUG_LOGF("Set user_sid3e to %" PRIu32 ".\n", new_user_sid3e);
}

uint32_t history_get_user_sid3e()
{
    return history_main_data.data_v1.user_sid3e;
}

void history_free()
{
    switch (history_main_data.save_version)
    {
        break; case 0:
        {
            if (save_format_0_free(&history_main_data.data_v0))
            {
                fprintf(stderr, ANSI_RED "Failed to free v0 data.\n" ANSI_RESET);
            }
        }
        break; case 1:
        {
            if (save_format_1_free(&history_main_data.data_v1))
            {
                fprintf(stderr, ANSI_RED "Failed to free v1 data.\n" ANSI_RESET);
            }
        }
    }
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
        char *autoexec_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v1.tf2_filepath), TF2PW_AUTOEXEC_SEMINAME);
        TF2_PLAYED_WITH_DEBUG_LOGF("Set autoexec_fullname to \"%s\".\n", autoexec_fullname);

        FILE *autoexec_handle = fopen(autoexec_fullname, "a");
        if (!autoexec_handle)
        {
            fprintf(stderr, ANSI_RED "Failed to open autoexec file \"%s\" for appending: ", autoexec_fullname);
            perror(NULL);
            ANSI_RESET_STDERR();
            free(autoexec_fullname);

            return true;
        }

        fprintf(autoexec_handle, "// BEGIN Generated automatically by TF2PW, don't edit.\ncon_logfile " TF2PW_LOG_FILENAME "\n// END   Generated automatically by TF2PW, don't edit.\n");

        if (fclose(autoexec_handle))
        {
            fprintf(stderr, ANSI_RED "Failed to close autoexec file \"%s\": ", autoexec_fullname);
            perror(NULL);
            ANSI_RESET_STDERR();
            free(autoexec_fullname);

            return true;
        }

        free(autoexec_fullname);
    }

    if (user_input_confirm("Replace W bind with forward and status? (Y/N): ", NULL))
    {
        #define TF2PW_CONFIG_SEMINAME TF2PW_CFG_SEMINAME "config.cfg"
        #define TF2PW_TEMP_SEMINAME TF2PW_CFG_SEMINAME "tf2pw.cfg.tmp"

        char *config_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v1.tf2_filepath), TF2PW_CONFIG_SEMINAME);

        FILE *config_handle = fopen(config_fullname, "r");
        if (!config_handle)
        {
            fprintf(stderr, ANSI_RED "Failed to open config file \"%s\" for appending: ", config_fullname);
            perror(NULL);
            ANSI_RESET_STDERR();
            free(config_fullname);

            return true;
        }

        // Search for `bind "w" "+forward"`. If exists, replace with `bind "w" "+forward ; status"`
        const char
            config_to_replace [] = "bind \"w\" \"+forward\"",
            config_replacement[] = "bind \"w\" \"+forward ; status\""
        ;

        char *temporary_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v1.tf2_filepath), TF2PW_TEMP_SEMINAME);
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
            ANSI_RESET_STDERR();

            free(config_fullname);
            free(temporary_fullname);

            return true;
        }

        if (fclose(config_handle))
        {
            fprintf(stderr, ANSI_RED "Failed to close config file \"%s\": ", config_fullname);
            perror(NULL);
            ANSI_RESET_STDERR();

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

    history_main_data.data_v1.tf2_live_log_fullname = cider_construct_fullname(string_deep_copy(history_main_data.data_v1.tf2_filepath), TF2PW_LOG_SEMINAME);
    TF2_PLAYED_WITH_DEBUG_LOGF("Set tf2_live_log_fullname to \"%s\".\n", history_main_data.data_v1.tf2_live_log_fullname);

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
            TF2_PLAYED_WITH_DEBUG_LOGF("Set user-sid3e to %" PRIu32 ".\n", new_user_sid3e);
            history_main_data.data_v1.user_sid3e = new_user_sid3e;
            break;
        }
    }

    history_main_data.data_v1.default_record_messages = user_input_confirm("Record chat messages by default (Y/N): ", NULL);

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

bool history_load(const char *const passed_history_fullname)
{
    const char *const history_fullname = (passed_history_fullname ? passed_history_fullname : history_get_default_fullname());

    TF2_PLAYED_WITH_DEBUG_LOGF("Loading history file \"%s\"\n", history_fullname);

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
                return true;
            }

            history_free();

            return history_save(history_fullname);
        }
        else
        {
            fprintf(stderr, ANSI_RED "Failed to open \"%s\" for reading. Error: ", history_fullname);
            perror(NULL);
            ANSI_RESET_STDERR();

            return true;
        }
    }

    history_free();

    bool retval = false;

    char header_buf[sizeof(HEADER) - 1];
    fread(header_buf, 1, sizeof(HEADER) - 1, input_file_ptr);
    if (strncmp(header_buf, HEADER, sizeof(HEADER) - 1))
    {
        fprintf(stderr, ANSI_RED "Requested file \"%s\" is not a valid tf2pw history file.\n" ANSI_RESET, history_fullname);

        retval = true;
        goto CLOSE_HISTORY_FILE;
    }

    fread_one(history_main_data.save_version);
    for (bool continue_modernizing = true, loaded = false; continue_modernizing; )
    {
        switch (history_main_data.save_version)
        {
            break; default:
            {
                fprintf(stderr, ANSI_RED "Version of history file \"%s\" is not supported by this version of TF2PW, get a newer version at " TF2PW_HOMEPAGE_URL ".\n" ANSI_RESET, history_fullname);

                retval = true;
                goto CLOSE_HISTORY_FILE;
            }
            break; case 0:
            {
                if (!loaded && save_format_0_load(&history_main_data.data_v0, input_file_ptr))
                {
                    fprintf(stderr, ANSI_RED "Failed to load file v0: \"%s\".\n" ANSI_RESET, history_fullname);

                    retval = true;
                    goto CLOSE_HISTORY_FILE;
                }
                else if (save_format_0_modernize(&history_main_data.data_v0))
                {
                    fprintf(stderr, ANSI_RED "Failed to modernize file v0: \"%s\".\n" ANSI_RESET, history_fullname);

                    retval = true;
                    goto CLOSE_HISTORY_FILE;
                }

                if (!retval)
                {
                    loaded = true;

                    TF2_PLAYED_WITH_DEBUG_LOGS("Successfully modernized v0 -> v1.\n");
                }
            }
            // Fallthrough
            case 1:
            {
                if (!loaded && save_format_1_load(&history_main_data.data_v1, input_file_ptr))
                {
                    fprintf(stderr, ANSI_RED "Failed to load file v1: \"%s\".\n" ANSI_RESET, history_fullname);

                    retval = true;
                    goto CLOSE_HISTORY_FILE;
                }
                else
                {
                    history_main_data.save_version = 1;
                }

                continue_modernizing = false;
            }
        }
    }

    if (EOF != fgetc(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "EOF not reached for file \"%s\". History file not valid, exiting.\n" ANSI_RESET, history_fullname);

        retval = true;
    }

    CLOSE_HISTORY_FILE:;
    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        ANSI_RESET_STDERR();

        return true;
    }

    return retval;
}

bool history_save(const char *const passed_history_fullname)
{
    const char *const history_fullname = (passed_history_fullname ? passed_history_fullname : history_get_default_fullname());

    TF2_PLAYED_WITH_DEBUG_LOGF("Saving history file \"%s\"\n", history_fullname);

    FILE *const output_file_ptr = fopen(history_fullname, "w");
    if (!output_file_ptr)
    {
        fprintf(stderr, ANSI_RED "Failed to open \"%s\" for writing. Error: ", history_fullname);
        perror(NULL);
        ANSI_RESET_STDERR();

        return true;
    }

    fwrite(HEADER, sizeof(char), sizeof(HEADER) - 1, output_file_ptr);

    const bool retval = save_format_1_save(&history_main_data.data_v1, output_file_ptr);
    if (retval)
    {
        fprintf(stderr, ANSI_RED "Failed to save file \"%s\".\n", history_fullname);
    }

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, ANSI_RED "Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        ANSI_RESET_STDERR();

        return true;
    }

    return retval;
}

// IMMED_TODO: Omit Team Fortress 2 from end in accordance with #67
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
        prealloc(new_tf2_filepath, new_tf2_filepath_len + 2);
        new_tf2_filepath[new_tf2_filepath_len] = CIDER_PATH_DELIM_C;
        new_tf2_filepath[new_tf2_filepath_len + 1] = '\0';
    }

    free(history_main_data.data_v1.tf2_filepath);
    history_main_data.data_v1.tf2_filepath = new_tf2_filepath;

    history_main_data.data_v1.tf2_filepath_len = (uint8_t) new_tf2_filepath_len;

    TF2_PLAYED_WITH_DEBUG_LOGF("Setting tf2_filepath to \"%s\".\n", history_main_data.data_v1.tf2_filepath);

    return history_main_data.data_v1.tf2_filepath;
}

const char *history_get_live_log_fullname()
{
    return (const char *) history_main_data.data_v1.tf2_live_log_fullname;
}

void history_set_date(const uint16_t new_date)
{
    if (new_date == HISTORY_SET_DATE_TODAY)
    {
        history_main_data.data_v1.current_date = time_manip_current_ued();
    }
    else
    {
        history_main_data.data_v1.current_date = new_date;
    }

    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        TF2_PLAYED_WITH_DEBUG_LOGS("Set current date to ");
        time_manip_print_ued(stderr, history_main_data.data_v1.current_date);
        fputs("\n" ANSI_RESET, stderr);
    )
}

static int compare_sid3e_player_sid3e(const struct player_record_0 *const player_record, const uint32_t *const current_sid3e)
{
    return (player_record->sid3e >= *current_sid3e ? (player_record->sid3e > *current_sid3e) : -1);
}

static int compare_date_player_date(const struct date_record_0 *const date_record, const uint64_t *const current_date)
{
    return (date_record->date >= *current_date ? (date_record->date > *current_date) : -1);
}

void history_add_record(const struct player_info *const pinfo)
{
    TF2_PLAYED_WITH_DEBUG_LOGF("Record add requested for (%s, %" PRIu32 ").\n", pinfo->name, pinfo->sid3e);

    const struct array_manip_find_return player_find_return = array_manip_find(&pinfo->sid3e, history_main_data.data_v1.player_records, sizeof(struct player_record_0), history_main_data.data_v1.player_records_len, (array_manip_find_compare_t) compare_sid3e_player_sid3e);
    switch (player_find_return.status)
    {
        break; case array_manip_find_status_found:
        {
            const struct array_manip_find_return date_find_return = array_manip_find(&history_main_data.data_v1.current_date, history_main_data.data_v1.player_records[player_find_return.index].date_records, sizeof(struct date_record_0), history_main_data.data_v1.player_records[player_find_return.index].date_records_len, (array_manip_find_compare_t) compare_date_player_date);
            switch (date_find_return.status)
            {
                break; case array_manip_find_status_found:
                {
                    ++history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].encounter_count;
                }
                break; case array_manip_find_status_prospective:
                {
                    prealloc(history_main_data.data_v1.player_records[player_find_return.index].date_records, ++history_main_data.data_v1.player_records[player_find_return.index].date_records_len);

                    if (date_find_return.index + 1 < history_main_data.data_v1.player_records[player_find_return.index].date_records_len)
                    {
                        memmove(history_main_data.data_v1.player_records[player_find_return.index].date_records + date_find_return.index + 2, history_main_data.data_v1.player_records[player_find_return.index].date_records + date_find_return.index + 1, sizeof(struct date_record_0) * (history_main_data.data_v1.player_records[player_find_return.index].date_records_len - player_find_return.index));
                    }

                    memcpy(
                        history_main_data.data_v1.player_records[player_find_return.index].date_records + player_find_return.index,
                        &(struct date_record_0)
                        {
                            .date = history_main_data.data_v1.current_date,
                            .encounter_count = 0,
                            .messages = NULL,
                            .messages_len = 0,
                        },
                        sizeof(struct date_record_0)
                    );

                    if (!strcmp(history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index - 1].name, pinfo->name))
                    {
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].name = history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index - 1].name;
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].name_len = 0;
                    }
                    else
                    {
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].name_len = (uint8_t) strlen(pinfo->name);
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].name = strcpy(malloc(history_main_data.data_v1.player_records[player_find_return.index].date_records[date_find_return.index].name_len + 1), pinfo->name);
                    }
                }
                break; case array_manip_find_status_start:
                {
                    prealloc(history_main_data.data_v1.player_records[player_find_return.index].date_records, ++history_main_data.data_v1.player_records[player_find_return.index].date_records_len);

                    memmove(history_main_data.data_v1.player_records[player_find_return.index].date_records + 1, history_main_data.data_v1.player_records[player_find_return.index].date_records, sizeof(struct date_record_0) * history_main_data.data_v1.player_records[player_find_return.index].date_records_len - 1);

                    memcpy(
                        history_main_data.data_v1.player_records[player_find_return.index].date_records,
                        &(struct date_record_0)
                        {
                            .date = history_main_data.data_v1.current_date,
                            .encounter_count = 0,
                            .messages = NULL,
                            .messages_len = 0,
                        },
                        sizeof(struct date_record_0)
                    );

                    if (history_main_data.data_v1.player_records[player_find_return.index].date_records_len == 1)
                    {
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name_len = (uint8_t) strlen(pinfo->name);
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name = strcpy(malloc(history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name_len + 1), pinfo->name);
                    }
                    else if (!strcmp(history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name, history_main_data.data_v1.player_records[player_find_return.index].date_records[1].name))
                    {
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name = history_main_data.data_v1.player_records[player_find_return.index].date_records[1].name;
                        history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name_len = 0;
                    }
                }
            }
        }
        break; case array_manip_find_status_prospective:
        {
            prealloc(history_main_data.data_v1.player_records, ++history_main_data.data_v1.player_records_len);
        }
        break; case array_manip_find_status_start:
        {
            prealloc(history_main_data.data_v1.player_records, ++history_main_data.data_v1.player_records_len);

            memmove(history_main_data.data_v1.player_records, history_main_data.data_v1.player_records + 1, sizeof(struct player_record_0) * (history_main_data.data_v1.player_records_len - 1));

            memcpy(
                history_main_data.data_v1.player_records + 0,
                &(struct player_record_0)
                {
                    .date_records = malloc(sizeof(struct date_record_0)),
                    .date_records_len = 1,
                    .notes = NULL,
                    .record_messages = history_main_data.data_v1.default_record_messages,
                    .sid3e = pinfo->sid3e
                },
                sizeof(struct player_record_0)
            );

            memcpy(
                history_main_data.data_v1.player_records[0].date_records + 0,
                &(struct date_record_0)
                {
                    .date = history_main_data.data_v1.current_date,
                    .encounter_count = 0,
                    .messages = NULL,
                    .messages_len = 0
                },
                sizeof(struct date_record_0)
            );

            history_main_data.data_v1.player_records[0].date_records[0].name_len = (uint8_t) strlen(pinfo->name);
            history_main_data.data_v1.player_records[0].date_records[0].name = strcpy(malloc(history_main_data.data_v1.player_records[player_find_return.index].date_records[0].name_len + 1), pinfo->name);
        }
    }
}

void history_print_record(const uint32_t requested_sid3e)
{
    const struct array_manip_find_return find_return = array_manip_find(&requested_sid3e, history_main_data.data_v1.player_records, sizeof(struct player_record_0), history_main_data.data_v1.player_records_len, (array_manip_find_compare_t) compare_sid3e_player_sid3e);
    if (find_return.status == array_manip_find_status_found)
    {
        printf("Records for requested player [U:1:%" PRIu32 "]:\n", requested_sid3e);

        if (history_main_data.data_v1.player_records[find_return.index].notes)
        {
            printf(LTAB "Notes:\n" LTAB LTAB "%s", history_main_data.data_v1.player_records[find_return.index].notes);
        }

        for (uint_fast32_t date_index = 0; date_index < history_main_data.data_v1.player_records[find_return.index].date_records_len; ++date_index)
        {
            printf(LTAB);
            time_manip_print_ued(stdout, history_main_data.data_v1.player_records[find_return.index].date_records[date_index].date);
            printf(":\n");

            printf(LTAB LTAB "Times encountered: %" PRIu8 "\n", history_main_data.data_v1.player_records[find_return.index].date_records[date_index].encounter_count + 1);
            printf(LTAB LTAB "Name: \"%s\"\n", history_main_data.data_v1.player_records[find_return.index].date_records[date_index].name);

            if (history_main_data.data_v1.player_records[find_return.index].record_messages && history_main_data.data_v1.player_records[find_return.index].date_records[date_index].messages)
            {
                printf(LTAB LTAB "Messages:\n");
                for (size_t msg_i = 0; msg_i < history_main_data.data_v1.player_records[find_return.index].date_records[date_index].messages_len; ++msg_i)
                {
                    printf(LTAB LTAB LTAB "%s\n", history_main_data.data_v1.player_records[find_return.index].date_records[date_index].messages[msg_i]);
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

    for (uint_fast32_t player_i = 0; player_i < history_main_data.data_v1.player_records_len; ++player_i)
    {
        for (uint_fast32_t date_i = 0; date_i < history_main_data.data_v1.player_records[player_i].date_records_len; ++date_i)
        {
            // If name is not pointer to previous copycat name, and matches requested name, print record of that player, continue to next player_i
            if (history_main_data.data_v1.player_records[player_i].date_records[date_i].name_len && !strcmp(history_main_data.data_v1.player_records[player_i].date_records[date_i].name, name))
            {
                record_found = true;
                history_print_record(history_main_data.data_v1.player_records[player_i].sid3e);
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
    const struct array_manip_find_return find_return = array_manip_find(&requested_sid3e, history_main_data.data_v1.player_records, sizeof(struct player_record_0), history_main_data.data_v1.player_records_len, (array_manip_find_compare_t) compare_sid3e_player_sid3e);
    if (find_return.status == array_manip_find_status_found)
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
        ANSI_RESET_STDERR();
        return;
    }

    if (history_main_data.data_v1.player_records[find_return.index].notes)
    {
        fputs(history_main_data.data_v1.player_records[find_return.index].notes, write);
        fputc('\n', write);
    }

    if (fclose(write))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp note-editing file: ");
        perror(NULL);
        ANSI_RESET_STDERR();
        return;
    }

    const char *const editor = getenv("EDITOR");

    char cmd_buff[128];
    sprintf(cmd_buff, "%s %s", (editor ? editor : "vi"), temporary_edit_file);
    system(cmd_buff);

    FILE *read = fopen(temporary_edit_file, "r");
    if (!read)
    {
        fprintf(stderr, ANSI_RED "Failed to open temp note-editing file: ");
        perror(NULL);
        ANSI_RESET_STDERR();
        return;
    }

    const size_t notes_len = file_io_buffered_input(read, &history_main_data.data_v1.player_records[find_return.index].notes);

    // If user entered nothing/deleted all notes, free and set to NULL
    if (notes_len == 0)
    {
        free(history_main_data.data_v1.player_records[find_return.index].notes);
        history_main_data.data_v1.player_records[find_return.index].notes = NULL;
    }
    else if (history_main_data.data_v1.player_records[find_return.index].notes[notes_len - 2] != '\n')
    {
        prealloc(history_main_data.data_v1.player_records[find_return.index].notes, notes_len + 1);
        history_main_data.data_v1.player_records[find_return.index].notes[notes_len - 1] = '\n';
        history_main_data.data_v1.player_records[find_return.index].notes[notes_len] = '\0';
    }

    if (fclose(read))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp note-editing file: ");
        perror(NULL);
        ANSI_RESET_STDERR();
        return;
    }

    if (remove(temporary_edit_file))
    {
        fprintf(stderr, ANSI_RED "Failed to delete temp note-editing file: ");
        perror(NULL);
        ANSI_RESET_STDERR();
        return;
    }

    free(temporary_edit_file);
}

void history_add_message(const uint32_t requested_sid3e, const char *const message)
{
    const struct array_manip_find_return find_return = array_manip_find(&requested_sid3e, history_main_data.data_v1.player_records, sizeof(struct player_record_0), history_main_data.data_v1.player_records_len, (array_manip_find_compare_t) compare_sid3e_player_sid3e);
    if (find_return.status == array_manip_find_status_found)
    {
        fprintf(stderr, ANSI_RED "Requested player SID3E(%" PRIu32 ") not found.\n" ANSI_RESET, requested_sid3e);
        return;
    }

    if (history_main_data.data_v1.player_records[find_return.index].record_messages)
    {
        // BSEARCH_TODO
        for (uint_fast32_t date_i = 0; date_i < history_main_data.data_v1.player_records[find_return.index].date_records_len; ++date_i)
        {
            if (history_main_data.data_v1.player_records[find_return.index].date_records[date_i].date == history_main_data.data_v1.current_date)
            {
                const size_t message_len = strlen(message) + 1;

                prealloc(history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages, ++history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages_len);
                history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages[history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages_len - 1] = malloc(sizeof(char) * message_len);
                memcpy(history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages[history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages_len - 1], message, message_len);

                TF2_PLAYED_WITH_DEBUG_LOGF("Message add requested: (%" PRIu32 ", \"%s\").\n", requested_sid3e, history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages[history_main_data.data_v1.player_records[find_return.index].date_records[date_i].messages_len - 1]);
                return;
            }
        }

        // Should have returned above. Getting here means there was no applicable record
        fprintf(stderr, "No applicable record in history_add_message(...) for SID3E=%" PRIu32 ".\n", requested_sid3e);
        abort();
    }
}
