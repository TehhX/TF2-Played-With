#ifndef FILE_IO_H
#define FILE_IO_H

/*
    file_io.h
    ---------

    Contains functionality for FILE handle input/output.
*/

#include "stdio.h"
#include "stdlib.h"

/*
    @brief Reads input from `stream` until EOF or one of passed characters in `stops`

        @param stream File stream to read from
        @param string String to place contents into
        @param stops List of characters to stop at, doesn't require null-termination as length is handled via `stops_len`
        @param stops_len Length of `stops`

        @returns Amount of characters written to `*string`

        @warning Return string is malloc'd and must be free'd by caller
        @warning Input `string` must be either malloc'd or NULL
        @warning Characters `stops[0]` to `stops[stops_len - 1]` will be read, ensure they all fall within `stops`'s jurisdiction to avoid UB
*/
extern size_t file_io_buffered_input(FILE *stream, char **string, const char *stops, size_t stops_len);

#endif // FILE_IO_H
