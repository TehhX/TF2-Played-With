#ifndef SAVE_FORMATS_MASTER_H
#define SAVE_FORMATS_MASTER_H

/*
    save_formats/master.h
    ---------------------

    Contains master definitions of all save format files, along with various definitions
    ------------------------------------------------------------------------------------

    All save file formats will need to be convertible to the latest version, however the ability for a later format to be convertible to an earlier version is not required. A save function will only exist for the latest version, but load functions will exist for all versions. All versions which are *not* the latest will have a function to convert their data to the latest version. To illustrate this, an example:
        * Version 0
            * Load
            * Convert to version 2

        * Version 1
            * Load
            * Convert to version 2

        * Version 2 (Latest)
            * Load
            * Save
            * User wizard

    In this example, if a version 3 is added, version 2 will lose the save and wizard functions, while the new v3 will have load, save, and user wizard functions. A list of requirements for adding a new format version:
        * Update SAVE_FORMAT_VERSION_LATEST
        * Remove save and wizard functions from previously latest version
        * Add new .[hc] files for the new version, add load, save, wizard functions

    Example prototypes for each functionality:
        * Load: bool load(struct save_format_data *data_output, FILE *load_stream);
            Accepts a save data pointer to write to, a file stream to load from, returns true if failure occurred

        * Save: bool save(const struct save_format_data *data_input, FILE *save_stream);
            Accepts a save data pointer to read from, a file stream to save to, returns true if failure occurred

        * Wizard: bool wizard(struct save_format_data *data_output)
            Accepts a save data pointer to write to, returns true if failure occurred

        * Conversion: bool modernize(struct save_format_data *data_input_output)
            Accepts a save data pointer to both read the current data from and write new data to, returns true if failure occurred

    The current format structure only allows for 256 [0, 255] unique format versions because it is only one unsigned byte. While it's likely the total amount of formats will not even come close to 256, it should be kept in mind if it does. Likely, another byte will be allocated, with a save format first byte value of 255 signifying a second byte should be checked eg. version 255 == 0xFF00, version 256 == 0xFF01, and so on
*/

#include "stdint.h"

// The header of every valid TF2PW file
#define HEADER "TF2PW"

// The latest available save format version
#define SAVE_FORMAT_VERSION_LATEST ((uint8_t) 0)

struct save_format_data
{
    uint8_t save_version;

    void *save_data;
};

#endif // SAVE_FORMATS_MASTER_H
