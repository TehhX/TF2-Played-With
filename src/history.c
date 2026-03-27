#include "history.h"

#include "common.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"
#include "stdio.h"
#include "time.h"

// Inter-unit variable defs
uint8_t history_initialized = 0;

// The header of any given tf2pw file. If not first 5 bytes of TF2PW file, it is invalid
#define HEADER (char [5]){ "TF2PW" }

// The latest available save format version. Remember to keep this updated
#define SAVE_VERSION_LATEST ((uint8_t) 0)

static uint16_t current_date;
static  uint8_t save_version;

// TODO: Consider arena allocation
static uint32_t player_records_len = 0;
static struct
{
    uint32_t sid3e;

    uint32_t date_records_len;
    struct
    {
        uint16_t date;

        uint8_t name_len;
        int8_t *name;

        // NOTE: Add 1 to get value
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

    history_initialized = 1;

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
    }

    free(player_records);
    player_records_len = 0;
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

    history_initialized = 0;
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

            fprintf(stderr, "No history file found at \"%s\". Use defaults? (Y/N): ", history_fullname);

            const char input_char = fgetc(stdin);
            if (input_char == 'y' || input_char == 'Y' || input_char == '\n')
            {
                save_version = SAVE_VERSION_LATEST;

                history_free_memory();

                return;
            }
            else
            {
                fputs("Either modify another history file or accept creation of a new file on next run.\n", stderr);
                TF2_PLAYED_WITH_DEBUG_ABEX();
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

    char header_buf[sizeof(HEADER)];
    fread(header_buf, 1, sizeof(HEADER), input_file_ptr);
    if (strncmp(header_buf, HEADER, sizeof(HEADER)))
    {
        fprintf(stderr, ANSI_RED "MAJOR: Requested file \"%s\" is not a valid tf2pw history file.\n" ANSI_RESET, history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    fread_one(save_version);
    fread_one(player_records_len);

    // TODO: Leaks memory here under unknown interactive mode circumstances. Investigate further
    player_records = malloc(player_records_len * sizeof(*player_records));
    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++ player_records_i)
    {
        fread_one(player_records[player_records_i].sid3e);
        fread_one(player_records[player_records_i].date_records_len);

        int8_t *last_real_name;

        player_records[player_records_i].date_records = malloc(sizeof(*player_records[player_records_i].date_records) * player_records[player_records_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fread_one(player_records[player_records_i].date_records[date_records_i].date);
            fread_one(player_records[player_records_i].date_records[date_records_i].encounter_count);
            fread_one(player_records[player_records_i].date_records[date_records_i].name_len);

            // Only read real names, else set ptr to original
            if (player_records[player_records_i].date_records[date_records_i].name_len)
            {
                player_records[player_records_i].date_records[date_records_i].name = malloc(sizeof(int8_t) * (player_records[player_records_i].date_records[date_records_i].name_len + 1));
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

    fwrite(HEADER, sizeof(char), sizeof(HEADER), output_file_ptr);

    fwrite_one(save_version);
    fwrite_one(player_records_len);

    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++player_records_i)
    {
        fwrite_one(player_records[player_records_i].sid3e);
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
        current_date = UES_TO_DAYS(time(NULL));
    }
    else
    {
        current_date = new_date;
    }

    // TODO: Log human-readable date as well
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

    player_records[player_records_i].date_records = reallocarray(player_records[player_records_i].date_records, ++player_records[player_records_i].date_records_len, sizeof(*player_records[player_records_i].date_records));

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

    ALLOCATE_NAME:

    const uint_fast8_t current_name_len = current_date_record.name_len = strlen(name);
    current_date_record.name = malloc(sizeof(int8_t) * current_name_len + 1);
    memcpy(current_date_record.name, name, current_name_len + 1);

    #undef current_date_record
}

void history_add_record(const struct player_info *const restrict pinfo)
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

    // BSEARCH_TODO
    for (uint_fast32_t player_records_i = 0; player_records_i < player_records_len; ++player_records_i)
    {
        if (player_records[player_records_i].sid3e != pinfo->sid3e)
        {
            continue;
        }

        // Found requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is in records");
        for (uint_fast32_t date_records_i = 0; date_records_i < player_records[player_records_i].date_records_len; ++date_records_i)
        {
            if (player_records[player_records_i].date_records[date_records_i].date != current_date)
            {
                continue;
            }

            TF2_PLAYED_WITH_DEBUG_LOGF(" on current_date %" PRIu16 ". Incrementing encounter count.\n", current_date);

            ++player_records[player_records_i].date_records[date_records_i].encounter_count;

            return;
        }

        // No record found for current_date
        TF2_PLAYED_WITH_DEBUG_LOGF(", but not on current_date %" PRIu16 ". Adding new date record.\n", current_date);
        history_add_date_record(player_records_i, pinfo->name);

        return;
    }

    // Couldn't find requested player
    TF2_PLAYED_WITH_DEBUG_LOGF(" is not in records. Adding new player and date records.\n");

    // Clean up previous color changes
    SET_COLOR(stdout, ANSI_RESET);

    player_records = reallocarray(player_records, ++player_records_len, sizeof(*player_records));
    player_records[player_records_len - 1].sid3e = pinfo->sid3e;
    player_records[player_records_len - 1].date_records_len = 0;
    player_records[player_records_len - 1].date_records = NULL;

    history_add_date_record(player_records_len - 1, pinfo->name);

    return;
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

    // BSEARCH_TODO
    for (uint_fast32_t player_i = 0; player_i < player_records_len; ++player_i)
    {
        if (player_records[player_i].sid3e == requested_sid3e)
        {
            printf("Records for requested player SID3E=%" PRIu32 ":\n", requested_sid3e);

            for (uint_fast32_t date_i = 0; date_i < player_records[player_i].date_records_len; ++date_i)
            {
                const struct tm *const record_date = localtime(&(time_t){ DAYS_TO_UES(player_records[player_i].date_records[date_i].date) });

                printf(LITERAL_TAB "%04d-%02d-%02d:\n", record_date->tm_year + 1900, record_date->tm_mon + 1, record_date->tm_mday);
                printf(LITERAL_TAB LITERAL_TAB "Times encountered: %" PRIu8 "\n", player_records[player_i].date_records[date_i].encounter_count + 1);
                printf(LITERAL_TAB LITERAL_TAB "Name: \"%s\"\n", player_records[player_i].date_records[date_i].name);
            }

            return;
        }
    }

    printf("Requested player SID3E(%" PRIu32 ") not found.\n", requested_sid3e);
}

void history_print_records(const char *name)
{
    // IMPL_TODO
}
