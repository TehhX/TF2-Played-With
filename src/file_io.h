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
    @brief Reads input from `stream` until EOF or newline

        @param stream File stream to read from
        @param string String to place contents into

        @returns Amount of characters written to `*string`

        @warning Return string is malloc'd and must be free'd by caller
        @warning Input `string` must be either malloc'd or NULL
*/
extern size_t file_io_buffered_input(FILE *stream, char **string);

#endif // FILE_IO_H
