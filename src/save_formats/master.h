#ifndef SAVE_FORMATS_MASTER_H
#define SAVE_FORMATS_MASTER_H

/*
    save_formats/master.h
    ---------------------

    Contains master definitions of all save format files, along with various definitions
    ------------------------------------------------------------------------------------

    Each save format should have the following functionality, where `<V>` is the specific version:

        bool save_format_<V>_wizard(struct save_format_<V> *save_data)
        bool save_format_<V>_save(const struct save_format_<V> *save_data, const char *file)
        bool save_format_<V>_load(struct save_format_<V> *save_data, const char *file)
        bool save_format_<V>_mastize(const struct save_format_<V> *data_input, struct save_format_master *data_output)
        bool save_format_<V>_demastize(const struct save_format_master *data_input, struct save_format_<V> *data_output)

    Each will return `true` if an error occurs, else false
*/

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

// The latest available save format version
#define SAVE_FORMAT_VERSION_LATEST ((uint8_t) 0)

// The header of any given tf2pw file
#define HEADER "TF2PW"

struct save_format_data
{
    uint8_t  save_version;
       void *save_data;
};

struct save_format_master
{
    uint16_t current_date;

    uint8_t  tf2_filepath_len;
       char *tf2_filepath;

    char *tf2_live_log_fullname;

    uint32_t user_sid3e;

    uint8_t default_record_messages;

    uint32_t player_records_len;
    struct
    {
        uint32_t sid3e;

        uint8_t record_messages;

        char *notes;

        uint32_t date_records_len;
        struct
        {
            uint16_t date;

            size_t   messages_len;
              char **messages;

            uint8_t  name_len;
               char *name;

            uint8_t encounter_count;
        }
        *date_records;
    }
    *player_records;
};

#endif // SAVE_FORMATS_MASTER_H
