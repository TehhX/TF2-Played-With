#include "history.h"

#include "common.h"
#include "time_manip.h"
#include "steamid_manip.h"
#include "user_input.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

// The header of any given tf2pw file. If not first 5 bytes of TF2PW file, it is invalid
#define HEADER "TF2PW"
#define HEADER_SIZE (sizeof(HEADER) - 1)

// The latest available save format version. Remember to keep this updated
#define SAVE_VERSION_LATEST ((uint8_t) 0)

static bool history_initialized = false;
static uint16_t  current_date;
static  uint8_t  history_live_log_location_len;

static     char *history_live_log_location;
static  uint8_t save_version;
static uint32_t user_sid3e;

// TODO: Consider arena allocation
static uint32_t player_records_len = 0;
static struct
{
    uint32_t sid3e;

    char *notes;

    uint32_t date_records_len;
    struct
    {
        uint16_t date;

        uint8_t  name_len;
           char *name;

        // NOTE: Add 1 to get actual value, it's 0 indexed
        uint8_t encounter_count;
    }
    *date_records;
}
*player_records;

// Fullname of the history file to read from/write to
static char *history_fullname;

void history_init(char *requested_history_fullname)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history double-init.\n" ANSI_RESET);
            abort();
        }
    )

    history_initialized = true;

    if (requested_history_fullname)
    {
        history_fullname = requested_history_fullname;
    }
    else
    {
        history_fullname = cider_construct_fullname(cider_data_filepath(), "tf2pw.sav");
    }

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: History initialized with history_fullname as \"%s\".\n" ANSI_RESET, history_fullname);
}

bool history_is_initialized()
{
    return history_initialized;
}

void history_set_user_sid3e(const uint32_t new_user_sid3e)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized set_user_sid3e.\n" ANSI_RESET);
            abort();
        }
    )

    user_sid3e = new_user_sid3e;

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set user_sid3e to %" PRIu32 ".\n" ANSI_RESET, user_sid3e);
}

uint32_t history_get_user_sid3e()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized get_user_sid3e.\n" ANSI_RESET);
            abort();
        }
    )

    return user_sid3e;
}

// Frees memory associated with history, sets `player_records_len` to 0
HYPER_MACRO void history_free_memory()
{
    if (!player_records_len)
    {
        TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Attempted history_free_memory while player_records_len == 0, ignoring request.\n" ANSI_RESET);
        return;
    }

    for (uint32_t player_i = 0; player_i < player_records_len; ++player_i)
    {
        for (uint32_t date_i = 0; date_i < player_records[player_i].date_records_len; ++date_i)
        {
            if (player_records[player_i].date_records[date_i].name_len)
            {
                free(player_records[player_i].date_records[date_i].name);
            }
        }

        free(player_records[player_i].date_records);
        free(player_records[player_i].notes);
    }

    free(player_records);
    free(history_live_log_location);
}

void history_free()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history double-free.\n" ANSI_RESET);
            abort();
        }
    )

    history_free_memory();

    history_initialized = false;
}

// Reads a single variable from input_file_ptr of size BYTES, places in VAR
#define fread_one(VAR) fread(&VAR, sizeof(VAR), 1, input_file_ptr)

// Reads an array from input_file_ptr of length ARR##_len
#define fread_arr(ARR) fread(ARR, sizeof(*(ARR)), ARR##_len, input_file_ptr)

void history_load()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized load.\n" ANSI_RESET);
            abort();
        }
    )

    FILE *const input_file_ptr = fopen(history_fullname, "r");
    if (input_file_ptr == NULL)
    {
        // No file, request to use default values
        if (errno == ENOENT)
        {
            errno = 0;

            char *input = NULL;

            if (user_input_confirm("No history file found. Start setup? (Y/N): "))
            {
                save_version = SAVE_VERSION_LATEST;

                USER_GET_TLP:;
                user_input_getline(&input, "Enter path to TF2 live-logfile (eg. .../tf/log.txt): ");
                if (input[0] == '\n')
                {
                    goto USER_GET_TLP;
                }

                for (history_live_log_location_len = 0; input[history_live_log_location_len] != '\0'; ++history_live_log_location_len);
                history_live_log_location = memcpy(malloc(history_live_log_location_len + 1), input, history_live_log_location_len);
                history_live_log_location[history_live_log_location_len] = '\0';

                USER_GET_SID3E:;
                user_input_getline(&input, "Enter your STEAMID as one of [STEAMID3|STEAMID3E|STEAMID64]: ");
                if (input[0] == '\n')
                {
                    goto USER_GET_SID3E;
                }

                const uint32_t new_user_sid3e = sidm_parse_sid3e(input, Esteamid_type_unknown);
                if (new_user_sid3e == SIDM_ERR_NAME || new_user_sid3e == SIDM_ERR_MISC)
                {
                    fprintf(stderr, ANSI_RED "Bad ID value. Try again.\n" ANSI_RESET);
                    goto USER_GET_SID3E;
                }
                else if (new_user_sid3e == SIDM_ERR_RNGE)
                {
                    fprintf(stderr, ANSI_RED "ID value too large. Try again.\n" ANSI_RESET);
                    goto USER_GET_SID3E;
                }
                else
                {
                    user_sid3e = new_user_sid3e;
                }

                free(input);

                history_free_memory();

                return;
            }
            else
            {
                free(input);
                fputs(ANSI_RED "Either modify another history file or accept setup of a new file. Exiting.\n" ANSI_RESET, stderr);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, ANSI_RED "MAJOR: Failed to open \"%s\" for reading. Error: ", history_fullname);
            perror(NULL);
            SET_COLOR(stderr, ANSI_RESET);
            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
    }

    char header_buf[HEADER_SIZE];
    fread(header_buf, 1, HEADER_SIZE, input_file_ptr);
    if (strncmp(header_buf, HEADER, HEADER_SIZE))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Requested file \"%s\" is not a valid tf2pw history file.\n" ANSI_RESET, history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    fread_one(save_version);
    fread_one(user_sid3e);
    fread_one(player_records_len);
    fread_one(history_live_log_location_len);

    history_live_log_location = malloc(history_live_log_location_len + 1);
    fread_arr(history_live_log_location);
    history_live_log_location[history_live_log_location_len] = '\0';

    // MAJOR_TODO: Leaks memory here under unknown interactive mode circumstances. Investigate further
    player_records = malloc(player_records_len * sizeof(*player_records));
    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++ player_records_i)
    {
        fread_one(player_records[player_records_i].sid3e);

        // BUFF_TODO
        player_records[player_records_i].notes = NULL;
        int input;
        size_t notes_len = 0;
        while ((input = fgetc(input_file_ptr)) != '\0')
        {
            player_records[player_records_i].notes = realloc(player_records[player_records_i].notes, sizeof(char) * ++notes_len);
            player_records[player_records_i].notes[notes_len - 1] = input;
        }

        player_records[player_records_i].notes = realloc(player_records[player_records_i].notes, sizeof(char) * notes_len + 1);
        player_records[player_records_i].notes[notes_len] = '\0';

        // If just '\0', set to NULL
        if (notes_len == 0)
        {
            free(player_records[player_records_i].notes);
            player_records[player_records_i].notes = NULL;
        }

        fread_one(player_records[player_records_i].date_records_len);

        char *last_real_name;

        player_records[player_records_i].date_records = malloc(sizeof(*player_records[player_records_i].date_records) * player_records[player_records_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fread_one(player_records[player_records_i].date_records[date_records_i].date);
            fread_one(player_records[player_records_i].date_records[date_records_i].encounter_count);
            fread_one(player_records[player_records_i].date_records[date_records_i].name_len);

            // Only read real names, else set ptr to original
            if (player_records[player_records_i].date_records[date_records_i].name_len)
            {
                player_records[player_records_i].date_records[date_records_i].name = malloc(sizeof(char) * (player_records[player_records_i].date_records[date_records_i].name_len + 1));
                fread_arr(player_records[player_records_i].date_records[date_records_i].name);
                player_records[player_records_i].date_records[date_records_i].name[player_records[player_records_i].date_records[date_records_i].name_len] = '\0';

                last_real_name = player_records[player_records_i].date_records[date_records_i].name;
            }
            else
            {
                player_records[player_records_i].date_records[date_records_i].name = last_real_name;
            }
        }
    }

    if (EOF != fgetc(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "MAJOR: EOF not reached for file \"%s\". Memory might be invalid.\n" ANSI_RESET, history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}

// Writes a single variable VAR to file stream output_file_ptr
#define fwrite_one(VAR) fwrite(&VAR, sizeof(VAR), 1, output_file_ptr)

// Writes an array to output_file_ptr of length ARR##_len
#define fwrite_arr(ARR) if (ARR) fwrite(ARR, sizeof(*ARR), ARR##_len, output_file_ptr)

void history_save()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized save.\n" ANSI_RESET);
            abort();
        }
    )

    FILE *const output_file_ptr = fopen(history_fullname, "w");
    if (!output_file_ptr)
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to open \"%s\" for writing. Error: ", history_fullname);
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    fwrite(HEADER, sizeof(char), HEADER_SIZE, output_file_ptr);

    fwrite_one(save_version);
    fwrite_one(user_sid3e);
    fwrite_one(player_records_len);
    fwrite_one(history_live_log_location_len);
    fwrite_arr(history_live_log_location);

    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++player_records_i)
    {
        fwrite_one(player_records[player_records_i].sid3e);

        // Only write notes if they exist, else just '\0'
        if (player_records[player_records_i].notes)
        {
            fprintf(output_file_ptr, "%s", player_records[player_records_i].notes);
        }

        putc('\0', output_file_ptr);

        fwrite_one(player_records[player_records_i].date_records_len);

        for (uint_fast32_t date_records_i = 0; date_records_i < player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fwrite_one(player_records[player_records_i].date_records[date_records_i].date);
            fwrite_one(player_records[player_records_i].date_records[date_records_i].encounter_count);
            fwrite_one(player_records[player_records_i].date_records[date_records_i].name_len);

            // Only write real names
            if (player_records[player_records_i].date_records[date_records_i].name_len)
            {
                fwrite_arr(player_records[player_records_i].date_records[date_records_i].name);
            }
        }
    }

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}

void history_set_live_log_location(char *live_log_location)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized set_live_log_location.\n" ANSI_RESET);
            abort();
        }
    )

    free(history_live_log_location);
    history_live_log_location = live_log_location;
    history_live_log_location_len = strlen(history_live_log_location);

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set live_log_location to \"%s\"." ANSI_RESET, history_live_log_location);
}

const char *history_get_live_log_location()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized get_live_log_location.\n" ANSI_RESET);
            abort();
        }
    )

    return history_live_log_location;
}

// MAJOR_TODO: Dates sometimes errantly set as 0 (epoch) during live-testing
void history_set_date(const uint16_t new_date)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized set_date.\n" ANSI_RESET);
            abort();
        }
    )

    if (new_date == HISTORY_SET_DATE_TODAY)
    {
        current_date = time_manip_ues2ued(time(NULL));
    }
    else
    {
        current_date = new_date;
    }

    // TODO: Log human-readable date instead of days
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set current_date to %" PRIu16 ".\n" ANSI_RESET, current_date);
}

HYPER_MACRO void history_add_date_record(const uint32_t player_records_i, const steam_name_stack name)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized add_date_record.\n" ANSI_RESET);
            abort();
        }
    )

    const uint_fast32_t date_records_i = player_records[player_records_i].date_records_len;
    #define current_date_record player_records[player_records_i].date_records[date_records_i]

    player_records[player_records_i].date_records = realloc(player_records[player_records_i].date_records, sizeof(*player_records[player_records_i].date_records) * ++player_records[player_records_i].date_records_len);

    current_date_record.date = current_date;
    current_date_record.encounter_count = 0;

    // This player record has date records aka. is not new
    if (date_records_i)
    {
        // Search backwards for same name
        for (uint_fast32_t date_name_search_i = date_records_i - 1; date_records_i != UINT_FAST32_MAX; --date_name_search_i)
        {
            // Previous date record is real
            if (player_records[player_records_i].date_records[date_name_search_i].name_len)
            {
                // If are the same, point this name to same place as last name
                if (!strcmp(player_records[player_records_i].date_records[date_name_search_i].name, name))
                {
                    current_date_record.name = player_records[player_records_i].date_records[date_name_search_i].name;
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
    const uint_fast8_t current_name_len = current_date_record.name_len = strlen(name);
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
    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++player_records_i)
    {
        if (player_records[player_records_i].sid3e == requested_sid3e)
        {
            return player_records_i;
        }
    }

    // Couldn't find
    return PLAYER_INDEX_ENOENT;
}

void history_add_record(const struct player_info *const pinfo)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized add_record.\n" ANSI_RESET);
            abort();
        }
    )

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Record add requested for (%s, %" PRIu32 "). Requested", pinfo->name, pinfo->sid3e);

    const uint_fast32_t player_index = get_player_index(pinfo->sid3e);
    if (player_index != PLAYER_INDEX_ENOENT)
    {
        // Found requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is in records");
        for (uint_fast32_t date_records_i = 0; date_records_i < player_records[player_index].date_records_len; ++date_records_i)
        {
            if (player_records[player_index].date_records[date_records_i].date != current_date)
            {
                continue;
            }

            TF2_PLAYED_WITH_DEBUG_LOGF(" on current_date %" PRIu16 ". Incrementing encounter count.\n", current_date);

            ++player_records[player_index].date_records[date_records_i].encounter_count;

            return;
        }

        // No record found for current_date
        TF2_PLAYED_WITH_DEBUG_LOGF(", but not on current_date %" PRIu16 ". Adding new date record.\n", current_date);
        history_add_date_record(player_index, pinfo->name);

        return;
    }
    else
    {
        // Couldn't find requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is not in records. Adding new player and date records.\n" ANSI_RESET);

        player_records = realloc(player_records, sizeof(*player_records) * ++player_records_len);
        player_records[player_records_len - 1].sid3e = pinfo->sid3e;
        player_records[player_records_len - 1].notes = NULL;
        player_records[player_records_len - 1].date_records_len = 0;
        player_records[player_records_len - 1].date_records = NULL;

        history_add_date_record(player_records_len - 1, pinfo->name);
    }
}

void history_print_record(const uint32_t requested_sid3e)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized print_record.\n" ANSI_RESET);
            abort();
        }
    )

    const uint_fast32_t player_index = get_player_index(requested_sid3e);
    if (player_index != PLAYER_INDEX_ENOENT)
    {
        printf("Records for requested player SID3E=%" PRIu32 ":\n", requested_sid3e);

        if (player_records[player_index].notes)
        {
            // TODO: Prints notes with multiple lines badly
            printf(LTAB "Notes:\n" LTAB LTAB "%s", player_records[player_index].notes);
        }

        for (uint_fast32_t date_i = 0; date_i < player_records[player_index].date_records_len; ++date_i)
        {
            printf(LTAB);
            time_manip_print_ued(player_records[player_index].date_records[date_i].date);
            printf(":\n");

            printf(LTAB LTAB "Times encountered: %" PRIu8 "\n", player_records[player_index].date_records[date_i].encounter_count + 1);
            printf(LTAB LTAB "Name: \"%s\"\n", player_records[player_index].date_records[date_i].name);
        }
    }
    else
    {
        printf("Requested player SID3E(%" PRIu32 ") not found.\n", requested_sid3e);
    }
}

void history_print_records(const char *const name)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized print_records.\n" ANSI_RESET);
            abort();
        }
    )

    for (uint_fast32_t player_i = 0; player_i < player_records_len; ++player_i)
    {
        for (uint_fast32_t date_i = 0; date_i < player_records[player_i].date_records_len; ++date_i)
        {
            // If name is not pointer to previous copycat name, and matches requested name, print record of that player, continue to next player_i
            if (player_records[player_i].date_records[date_i].name_len && !strcmp(player_records[player_i].date_records[date_i].name, name))
            {
                history_print_record(player_records[player_i].sid3e);
                continue;
            }
        }
    }
}

void history_edit_notes(uint32_t requested_sid3e)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, ANSI_RED "FATAL: Attempted history uninitialized edit_notes.\n" ANSI_RESET);
            abort();
        }
    )

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
        fprintf(stderr, ANSI_RED "Failed to open temp file: ");
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        return;
    }

    if (player_records[player_index].notes)
    {
        fputs(player_records[player_index].notes, write);
        fputc('\n', write);
    }

    if (fclose(write))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp file: ");
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
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
        fprintf(stderr, ANSI_RED "Failed to open temp file: ");
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        return;
    }

    // BUFF_TODO
    int input;
    size_t notes_len = 0;
    while ((input = fgetc(read)) != EOF)
    {
        player_records[player_index].notes = realloc(player_records[player_index].notes, sizeof(char) * (notes_len + 1));
        player_records[player_index].notes[notes_len++] = input;
    }

    if (notes_len > 0)
    {
        // TODO: Add newline if there is none at EOF
        player_records[player_index].notes[notes_len - 1] = '\0';
    }
    // If user entered nothing/deleted all notes, free and set to NULL
    else
    {
        free(player_records[player_index].notes);
        player_records[player_index].notes = NULL;
    }

    if (fclose(read))
    {
        fprintf(stderr, ANSI_RED "Failed to close temp file: ");
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        return;
    }

    if (remove(temporary_edit_file))
    {
        fprintf(stderr, ANSI_RED "Failed to delete temp file: ");
        perror(NULL);
        SET_COLOR(stderr, ANSI_RESET);
        return;
    }

    free(temporary_edit_file);
}
