#include "history.h"

#include "cider.h"

#include "stdint.h"
#include "inttypes.h"
#include "string.h"

int history_initialized = 0;

static char *history_fullname;

// Save contents in memory
static struct
{
    struct
    {
        uint_fast8_t
            save_format_version,
            user_steam_name_length
        ;

        char *user_steam_name;

        uint_fast32_t record_len;
    }
    header;

    struct
    {
        uint64_t steamid64;

        struct
        {
            uint_fast16_t date;

            uint_fast8_t name_len;

            char *name;

            uint_fast8_t encounters;
        }
        *date_record;
    }
    *record;
}
history_memory = { 0 };

void history_init(int file_num)
{
    TF2_PLAYED_WITH_DEBUG_INSERT
    (
        if (history_initialized)
        {
            fprintf(stderr, "FATAL: Attempted history double-init.\n");
            abort();
        }

        if (file_num < 0 || file_num > 99)
        {
            fprintf(stderr, "FATAL: Attempted history init while file_num=%d (out of range [0, 99]).\n", file_num);
            abort();
        }

        printf("History initialized: file_num = %d\n", file_num);
    )

    history_initialized = 1;

    char history_filename[sizeof("tf2pwXX.sav")];
    sprintf(history_filename, "tf2pw%s%d.sav", (file_num < 10 ? "0" : ""), file_num);
    TF2_PLAYED_WITH_DEBUG_LOGF("History filename: %s\n", history_filename);

    history_fullname = cider_construct_fullname(cider_data_filepath(), history_filename);
    TF2_PLAYED_WITH_DEBUG_LOGF("History fullname: \"%s\"\n", history_fullname);
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

    // TODO
}

// Writes a single variable VAR to file stream STREAM
#define fwrite_one(VAR, STREAM) fwrite(&VAR, sizeof(VAR), 1, STREAM)

// Writes an array ARR of length LEN to stream STREAM
#define fwrite_arr(ARR, LEN, STREAM) fwrite(ARR, sizeof(*ARR), LEN, STREAM)

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

    // Open file
    FILE *const output_file_ptr = fopen(history_fullname, "w");
    if (!output_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error", history_fullname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    // Write header
    fputs("TF2PW", output_file_ptr);
    fwrite_one(history_memory.header.save_format_version, output_file_ptr);

    // Close file
    if (fclose(output_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

void history_collect_live(const char *collections_fullname)
{
    // TODO
}

void history_collect_archived(const char *collections_fullname)
{
    // TODO
}
