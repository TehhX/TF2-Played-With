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
    uint_fast8_t save_format_version;

    uint_fast64_t user_steamid64;

    uint_fast32_t player_records_len;
    struct
    {
        uint_fast64_t record_steamid64;

        uint_fast32_t date_records_len;
        struct
        {
            uint_fast16_t date;

            uint_fast8_t name_len;
            int_fast8_t *name;

            uint_fast8_t encounter_count;
        }
        *date_records;
    }
    *player_records;
}
history_memory;

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

// Writes a single variable VAR of size BYTES to file stream STREAM
#define fwrite_one(VAR, BYTES, STREAM) fwrite(&VAR, BYTES, 1, STREAM)

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

    // Byte counts of each variable can be found in /README.md#structure
    fputs("TF2PW", output_file_ptr);

    fwrite_one(history_memory.save_format_version, 1, output_file_ptr);

    fwrite_one(history_memory.user_steamid64, 8, output_file_ptr);

    fwrite_one(history_memory.player_records_len, 4, output_file_ptr);

    for (uint_fast32_t i = 0; i < history_memory.player_records_len; ++i)
    {
        fwrite_one(history_memory.player_records[i].record_steamid64, 8, output_file_ptr);

        fwrite_one(history_memory.player_records[i].date_records_len, 4, output_file_ptr);

        for (uint_fast32_t o = 0; o < history_memory.player_records[i].date_records_len; ++o)
        {
            fwrite_one(history_memory.player_records[i].date_records[o].date, 2, output_file_ptr);

            fwrite_one(history_memory.player_records[i].date_records[o].name_len, 1, output_file_ptr);
            fwrite(history_memory.player_records[i].date_records[o].name, 1, history_memory.player_records[i].date_records[o].name_len, output_file_ptr);

            fwrite_one(history_memory.player_records[i].date_records[o].encounter_count, 1, output_file_ptr);
        }
    }

    // Close file
    if (fclose(output_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error", history_fullname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

#undef fwrite_one

void history_collect_live(const char *collections_fullname)
{
    // TODO
}

void history_collect_archived(const char *collections_fullname)
{
    // TODO
}
