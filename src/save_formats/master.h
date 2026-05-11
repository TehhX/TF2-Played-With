#ifndef SAVE_FORMATS_MASTER_H
#define SAVE_FORMATS_MASTER_H

/*
    save_formats/master.h
    ---------------------

    Contains master definitions of all save format files, along with various definitions.
*/

#include "stdbool.h"
#include "stdint.h"

// The latest available save format version
#define SAVE_VERSION_LATEST ((uint8_t) 0)

// The header of any given tf2pw file
#define HEADER "TF2PW"

struct save_format_data
{
    uint8_t save_version;

    void *save_data;
};

struct save_format_master
{
    // IMPL_TODO: Should contain one of every data of every save format for conversions
};

#endif // SAVE_FORMATS_MASTER_H
