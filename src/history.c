#include "history.h"

int history_initialized = 0;

static int save_location_index;

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

        printf("History initialized: file_num=%d\n", file_num);
    )

    history_initialized = 1;

    save_location_index = file_num;

    // TODO
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

    // TODO
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

    // TODO
}
