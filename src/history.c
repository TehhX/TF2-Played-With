#include "history.h"

#include "common.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"
#include "stdio.h"

// Sorted "cleaner" variables in header file, these are just the definitions
int8_t *names; uint8_t save_format_version, *name_lens, *encounter_counts, history_initialized = 0; uint16_t *dates, current_date; uint32_t user_steamid3_excerpt, player_records_len, *steam_id3_excerpts, *date_records_lens; size_t date_records_lens_len, name_lens_len, steam_id3_excerpts_len, dates_len, names_len, encounter_counts_len;

// Fullname of the history file to read from/write to
static char *history_fullname;

// The latest available save format version. Remember to keep this updated
#define SAVE_FORMAT_LATEST ((uint8_t) 0)

void history_init(char *requested_history_fullname)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (history_initialized)
        {
            fprintf(stderr, "FATAL: Attempted history double-init.\n");
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

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: History initialized with history_fullname as \"%s\".\n", history_fullname);
}

void history_free()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, "FATAL: Attempted history double-free.\n");
            abort();
        }
    )

    history_initialized = 0;

    free(history_fullname);
    free(date_records_lens);
    free(name_lens);
    free(steam_id3_excerpts);
    free(dates);
    free(names);
    free(encounter_counts);
}

#define STEAMID3_MAX "[U:1:4294967295]"
#define STEAMID3_START (char [5]){ "[U:1:" }

// Reads a single variable from input_file_ptr of size BYTES, places in VAR
#define fread_one(VAR) fread(&VAR, sizeof(VAR), 1, input_file_ptr)

// Allocates space for, reads in array of date from input_file_ptr. Requires previously defined variable ARR##_len to specify length of array to be allocated and populated
#define arr_allocread(ARR) ARR = malloc(sizeof(*ARR) * ARR##_len); fread(ARR, sizeof(*ARR), ARR##_len, input_file_ptr)

// The header of any given tf2pw file. If not, file is invalid
#define HEADER (char [5]){ "TF2PW" }

void history_load()
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (!history_initialized)
        {
            fprintf(stderr, "FATAL: Attempted history uninitialized load.\n");
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
                save_format_version = SAVE_FORMAT_LATEST;

                player_records_len     =
                date_records_lens_len  =
                name_lens_len          =
                steam_id3_excerpts_len =
                dates_len              =
                names_len              =
                encounter_counts_len   = 0;

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
            fprintf(stderr, "MAJOR: Failed to open \"%s\" for reading. Error: ", history_fullname);
            perror(NULL);
            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
    }

    char header_buf[sizeof(HEADER)];
    fread(header_buf, 1, sizeof(HEADER), input_file_ptr);
    if (strncmp(header_buf, HEADER, sizeof(HEADER)))
    {
        fprintf(stderr, "MAJOR: Requested file \"%s\" is not a valid tf2pw history file.\n", history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    fread_one(save_format_version);
    fread_one(user_steamid3_excerpt);
    fread_one(player_records_len);

    date_records_lens_len = player_records_len;
    arr_allocread(date_records_lens);

    size_t sum_date_records_lens = 0;
    for (uint32_t i = 0; i != date_records_lens_len; ++i)
    {
        sum_date_records_lens += date_records_lens[i];
    }

    name_lens_len = player_records_len * sum_date_records_lens;
    arr_allocread(name_lens);

    size_t sum_name_lens = 0;
    for (size_t i = 0; i < name_lens_len; ++i)
    {
        sum_name_lens += name_lens[i];
    }

    steam_id3_excerpts_len = player_records_len;
    arr_allocread(steam_id3_excerpts);

    dates_len = sum_date_records_lens;
    arr_allocread(dates);

    names_len = sum_name_lens;
    arr_allocread(names);

    encounter_counts_len = sum_name_lens;
    arr_allocread(encounter_counts);

    if (EOF != fgetc(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: EOF not reached for file \"%s\". Memory might be invalid.\n", history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
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
            fprintf(stderr, "FATAL: Attempted history uninitialized save.\n");
            abort();
        }
    )

    FILE *const output_file_ptr = fopen(history_fullname, "w");
    if (!output_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error: ", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    fwrite(HEADER, sizeof(char), sizeof(HEADER), output_file_ptr);

    fwrite_one(save_format_version);
    fwrite_one(user_steamid3_excerpt);
    fwrite_one(player_records_len);

    fwrite_arr(date_records_lens);
    fwrite_arr(name_lens);
    fwrite_arr(steam_id3_excerpts);
    fwrite_arr(dates);
    fwrite_arr(names);
    fwrite_arr(encounter_counts);

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error: ", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}

void history_add_record(const struct player_info *const restrict pinfo)
{
    TF2_PLAYED_WITH_DEBUG_LOGF("(%s, %" PRIu32 ")", pinfo->name, pinfo->sid3e);

    // BSEARCH_TODO
    for (size_t player_i = 0, dates_i; player_i < steam_id3_excerpts_len; ++player_i)
    {
        if (steam_id3_excerpts[player_i] != pinfo->sid3e)
        {
            continue;
        }

        // Found requested player
        TF2_PLAYED_WITH_DEBUG_LOGF(" is already in records.\n");
    }

    // Couldn't find requested player
    TF2_PLAYED_WITH_DEBUG_LOGF(" is not in records.\n");
}
