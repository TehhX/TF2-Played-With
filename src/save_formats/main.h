#ifndef SAVE_FORMATS_MAIN_H
#define SAVE_FORMATS_MAIN_H

/*
    save_formats/main.h
    ---------------------

    Contains main definitions of all save format files, along with various definitions
    ------------------------------------------------------------------------------------

    All save file formats will need to be convertible to the latest version by converting through each version individually, however the ability for a later format to be convertible to an earlier version is not required. A save function will only exist for the latest version, but load functions will exist for all versions. All versions which are *not* the latest will have a function to convert their data to the latest version. To illustrate this, an example:
        * Version 0
            * Load
            * Convert to version 1

        * Version 1
            * Load
            * Convert to version 2

        * Version 2 (Latest)
            * Load
            * Save

    In this example, if a version 3 is added, version 2 will lose the save function, while the new v3 will have load and save functions.

    A list of requirements for adding a new format version:
        * Update SAVE_FORMAT_VERSION_LATEST
        * Comment out save function from previously latest version
        * Add new .[hc] files for the new version, add load and save functions

    Example prototypes for each functionality:
        * Load: bool load(struct save_format_<V> *save_data, FILE *load_stream);
            Loads the file from load_stream into save_data. Accepts a save data pointer to write to, a file stream to load from, returns pointer to save_data if succeeded, else NULL

        * Save: bool save(const struct save_format_<V> *save_data, FILE *save_stream);
            Saves save_data to save_stream. Accepts a save data pointer to read from, a file stream to save to, returns true if failure occurred

        * Conversion: bool modernize(void *data_input_output)
            Converts save_format_<N> to save_format_<N + 1> in place. Accepts a save data pointer to both read the current data from and write new data to, returns true if failure occurred

        * Free: bool free(struct save_format_<V> *save_data)
            Frees memory associated with save_data.

    The current format structure only allows for 256 [0, 255] unique format versions because it is only one unsigned byte. While it's likely the total amount of formats will not even come close to 256, it should be kept in mind if it does. Likely, another byte will be allocated, with a save format first byte value of 255 signifying a second byte should be checked eg. version 255 == 0xFF00, version 256 == 0xFF01, and so on

    Only the latest save version will be usable in the program, older version specifications will only exist to modernize old files. Only the latest version will have a wizard under history.[hc]

    If a loaded file has a save version above the latest version, TF2PW should tell the user to update, and then exit.
*/

#include "stdint.h"
#include "stdbool.h"

#include "../common.h"
#include "../history.h"
#include "version_0.h"
#include "version_1.h"

// The header of every valid TF2PW file
#define HEADER "TF2PW"

// The latest available save format version
#define SAVE_FORMAT_VERSION_LATEST ((uint8_t) 1)

// Reads a single variable from input_file_ptr of size BYTES, places in VAR
#define fread_one(VAR) fread(&VAR, sizeof(VAR), 1, input_file_ptr)

// Reads an array from input_file_ptr of length ARR##_len
#define fread_arr(ARR) fread(ARR, sizeof(*(ARR)), ARR##_len, input_file_ptr)

// Writes a single variable VAR to file stream output_file_ptr
#define fwrite_one(VAR) fwrite(&VAR, sizeof(VAR), 1, output_file_ptr)

// Writes an array to output_file_ptr of length ARR##_len
#define fwrite_arr(ARR) if (ARR) fwrite(ARR, sizeof(*ARR), ARR##_len, output_file_ptr)

struct save_format_data
{
    uint8_t save_version;

    union
    {
        struct save_format_0 data_v0;
        struct save_format_1 data_v1;
    };
};

#endif // SAVE_FORMATS_MAIN_H
