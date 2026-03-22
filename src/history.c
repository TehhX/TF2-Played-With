#include "history.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "errno.h"

int history_initialized = 0;
static char *history_fullname;

// Refer to /README.md#structure for variable descriptions. Same order as seen there as well
static  uint8_t  save_format_version;
static uint32_t  user_steamid3_excerpt;
static uint32_t  player_records_len;
static uint32_t *date_records_lens;
static  uint8_t *name_lens;
static uint32_t *steam_id3_excerpts;
static uint16_t *dates;
static   int8_t *names;
static  uint8_t *encounter_counts;

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

    TF2_PLAYED_WITH_DEBUG_LOGF("History initialized with history_fullname as \"%s\".\n", history_fullname);
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
}

#define STEAMID3_MAX "[U:1:4294967295]"
#define STEAMID3_START (char [5]){ "[U:1:" }

// Populates memory with default history values via user input
static void history_populate()
{
    save_format_version = 0;

    printf("Enter your STEAMID3 or SID3 excerpt: ");

    char user_input_sid3e[sizeof(STEAMID3_MAX)];
    fgets(user_input_sid3e, sizeof(STEAMID3_MAX), stdin);
    for (int i = 0; i < sizeof(STEAMID3_MAX); ++i)
    {
        if (user_input_sid3e[i] == '\n')
        {
            user_input_sid3e[i] = '\0';
            break;
        }
    }

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: Entered STEAMID3: \"%s\"\n", user_input_sid3e);

    if (!strncmp(user_input_sid3e, STEAMID3_START, sizeof(STEAMID3_START)))
    {
        memmove(user_input_sid3e, user_input_sid3e + sizeof(STEAMID3_START), sizeof(STEAMID3_MAX) - sizeof(STEAMID3_START));

        for (int i = 0; i < sizeof(STEAMID3_MAX); ++i)
        {
            if (user_input_sid3e[i] == ']')
            {
                user_input_sid3e[i] = '\0';
                break;
            }
        }
    }

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: Final STEAMID3 excerpt: \"%s\"\n", user_input_sid3e);

    // TODO
}

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
        // No file, populate one
        if (errno == ENOENT)
        {
            printf("No history file found at specified location. Continue to first-time setup? (y/n): ");

            const int input_char = fgetc(stdin);
            if (input_char == 'y' || input_char == 'Y' || input_char == '\n')
            {
                history_populate();
                return;
            }
            else
            {
                fputs("First-time setup declined, either modify another history file or accept on next run.\n", stderr);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "MAJOR: Failed to open \"%s\" for reading. Error", history_fullname);
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

    // Keep len vairables for now, might need to be globalized later
    const size_t date_records_lens_len = player_records_len;
    arr_allocread(date_records_lens);

    size_t sum_date_records_lens = 0;
    for (uint32_t i = 0; i != date_records_lens_len; ++i)
    {
        sum_date_records_lens += date_records_lens[i];
    }

    const size_t name_lens_len = player_records_len * sum_date_records_lens;
    arr_allocread(name_lens);

    size_t sum_name_lens = 0;
    for (size_t i = 0; i < name_lens_len; ++i)
    {
        sum_name_lens += name_lens[i];
    }

    const size_t steam_id3_excerpts_len = player_records_len;
    arr_allocread(steam_id3_excerpts);

    const size_t dates_len = sum_date_records_lens;
    arr_allocread(dates);

    const size_t names_len = sum_name_lens;
    arr_allocread(names);

    const size_t encounter_counts_len = sum_name_lens;
    arr_allocread(encounter_counts);

    if (EOF != fgetc(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: EOF not reached for file \"%s\". Memory might be invalid.\n", history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}

// Writes a single variable VAR of size BYTES to file stream output_file_ptr
#define fwrite_one(VAR) fwrite(&VAR, sizeof(VAR), 1, output_file_ptr)

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
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    // TODO

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}

void history_collect_live(const char *collections_fullname)
{
    // TODO: Write
}

void history_collect_archived(const char *collections_fullname)
{
    FILE *const input_file_ptr = fopen(collections_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error\n", collections_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    // TODO: Write

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", collections_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}
