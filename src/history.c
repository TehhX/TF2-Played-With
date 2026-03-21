#include "history.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"

int history_initialized = 0;
static char *history_fullname;

static  uint_fast8_t save_format_version; // The version of the save format
static uint_fast32_t user_steamid3_excerpt; // The user's STEAMID3 excerpt
static uint_fast32_t player_records_len; // How many player records or unique other players there are

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

// Reads a single variable from input_file_ptr of size BYTES, places in VAR
#define fread_one(VAR, BYTES) fread(&VAR, BYTES, 1, input_file_ptr)

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

    FILE *const input_file_ptr = fopen(history_fullname, "w");
    if (!input_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for reading. Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }

    char header_buf[sizeof("TF2PW") - 1];
    fread(header_buf, 1, sizeof("TF2PW") - 1, input_file_ptr);
    if (strncmp(header_buf, "TF2PW", sizeof("TF2PW") - 1)) // Don't compare the null-term, str!N!cmp required for count - 1
    {
        fprintf(stderr, "MAJOR: Requested file \"%s\" is not a valid tf2pw history file.\n", history_fullname);
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }

    fread_one(  save_format_version, sizeof( uint8_t));
    fread_one(user_steamid3_excerpt, sizeof(uint32_t));
    fread_one(   player_records_len, sizeof(uint32_t));

    // TODO: Continue

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }
}

// Writes a single variable VAR of size BYTES to file stream output_file_ptr
#define fwrite_one(VAR, BYTES) fwrite(&VAR, BYTES, 1, output_file_ptr)

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
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }

    // TODO: Continue

    if (fclose(output_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX;
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
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }

    // TODO: Write

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", collections_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX;
    }
}
