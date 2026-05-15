#include "file_io.h"

#include "common.h"

#include "stdlib.h"
#include "stdio.h"

// How many bytes to read per cycle
#define READ_BYTES ((size_t) 64)

size_t file_io_buffered_input(FILE *stream, char **const string)
{
    for (size_t total_bytes = 0; 1; )
    {
        prealloc(*string, total_bytes + READ_BYTES);

        const size_t current_bytes = fread(*string + total_bytes, sizeof(char), READ_BYTES, stream);

        total_bytes += current_bytes;

        for (size_t last_offset = 0; last_offset < current_bytes; ++last_offset)
        {
            const size_t last_char_index = total_bytes - current_bytes + last_offset;

            if ((*string)[last_char_index] == '\n')
            {
                // Might need to clearerr here in case it parses EOF while not on last line

                // Reset back to newline
                fseek(stream, last_offset - current_bytes + 1, SEEK_CUR);

                prealloc(*string, last_char_index + 1);

                (*string)[last_char_index] = '\0';

                return last_char_index;
            }
        }

        if (current_bytes != READ_BYTES)
        {
            prealloc(*string, total_bytes + 1);

            (*string)[total_bytes] = '\0';

            return total_bytes;
        }
    }

    TF2_PLAYED_WITH_DEBUG_LOGS("Aborting on no-return file_io_buffered_input.\n");
    TF2_PLAYED_WITH_DEBUG_ABEX();
}
