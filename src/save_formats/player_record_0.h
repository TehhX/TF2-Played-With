#ifndef SAVE_FORMATS_PLAYER_RECORD_0_H
#define SAVE_FORMATS_PLAYER_RECORD_0_H

/*
    save_formats/player_record_0.h
    -------------------------------

    Contains first struct versions of player and date records used in save versions 0, 1
*/

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

struct date_record_0
{
    uint16_t date;

    size_t   messages_len;
      char **messages;

    uint8_t  name_len;
       char *name;

    uint8_t encounter_count;
};

struct player_record_0
{
    uint32_t sid3e;

    uint8_t record_messages;

    char *notes;

    uint32_t date_records_len;
    struct date_record_0 *date_records;
};

extern bool date_record_0_move(struct date_record_0 *dest, const struct date_record_0 *source);

extern bool player_record_0_move(struct player_record_0 *dest, const struct player_record_0 *source);

#endif // SAVE_FORMATS_PLAYER_RECORD_0_H
