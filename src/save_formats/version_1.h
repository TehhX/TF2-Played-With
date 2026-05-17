#ifndef SAVE_FORMATS_VERSION_1_H
#define SAVE_FORMATS_VERSION_1_H

/*
    save_formats/version_1.h
    --------------------------

    Contains functionality for version 1 of saving.

    Format Version 1 Structure
        Header
            |          Name           |                                                               Description                                                                |                 Size (Bytes)                  |                          Example                           |
            |:-----------------------:|:----------------------------------------------------------------------------------------------------------------------------------------:|:---------------------------------------------:|:----------------------------------------------------------:|
            |         Header          |                                     Header of the file format. Always "TF2PW", else file is invalid                                      |                       5                       |                      (char *) "TF2PW"                      |
            |   Save Format Version   |                                    The version of history file format used with this particular file                                     |                       1                       |                           (u8) 0                           |
            |       User SID3E        |                                                     The STEAMID3 excerpt of the user                                                     |                       4                       |                       (u32) 12345678                       |
            | Default Record Messages |                                The default value for Player Record::Record Messages of new player records                                |                       1                       |                           (u8) 0                           |
            |   TF2 Filepath Length   |                                         How many characters are in the Team Fortress 2 filepath                                          |                       1                       |                          (u8) 47                           |
            |      TF2 Filepath       |                                           The Team Fortress 2 filepath sans /Team Fortress 2/                                            |              TF2 Filepath Length              | (char *) "/home/Timmy/.local/share/Steam/steamapps/common" |
            |  Player Records Length  |                                     How many unique player records there are in the following array                                      |                       4                       |                        (u32) 12,000                        |
            |     Player Records      | An array of player records. Sorted by (Player Record::STEAMID3 Excerpt). See [Player Record](#player-record) for its particular contents | Player Records Length * sizeof(Player Record) |                            N/A                             |

        Player Record
            |        Name         |                                                        Description                                                         |               Size (Bytes)                |                 Example                 |
            |:-------------------:|:--------------------------------------------------------------------------------------------------------------------------:|:-----------------------------------------:|:---------------------------------------:|
            |  STEAMID3 Excerpt   |                                                    This player's SID3E                                                     |                     4                     |               (u32) 22202               |
            |   Record Messages   | Flag to check chat messages. 1 for yes, 0 for no. Will inherit value of Header::Record Messages until individually changed |                     1                     |                 (u8) 1                  |
            |        Notes        |                                  Notes taken by the user on this player. Null terminated                                   |                 Variable                  | (char *) "They were very nice to me.\0" |
            | Date Records Length |                                           How many date records this player has                                            |                     4                     |                (u32) 13                 |
            |    Date Records     |                   An array of date records. See [Date Record](#date-record) for its particular contents                    | Date Records Length * sizeof(Date Record) |                   N/A                   |

        Date Record
            |      Name       |                                                  Description                                                   | Size (Bytes) |               Example               |
            |:---------------:|:--------------------------------------------------------------------------------------------------------------:|:------------:|:-----------------------------------:|
            |      Date       |                                  How many days since UNIX epoch this date was                                  |      2       |                20537                |
            | Encounter Count |                      How many times this player was encountered on this date (minus one)                       |      1       |               (u8) 4                |
            |   Name Length   |                                        The length of the following name                                        |      1       |               (u8) 13               |
            |      Name       |                                      The name of the player on this date                                       | Name_Length  |          (char *) "Timmy"           |
            |    Messages     | Will only exist if Record Messages in Player Record is 1. Contains messages sent from this player on this date |   Variable   | (char *) "Wow!\nCool!\nGood job!\0" |
*/

#include "player_record_0.h"

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"

struct save_format_1
{
    uint16_t current_date;
    char *tf2_live_log_fullname;

    uint32_t user_sid3e;

    uint8_t default_record_messages;

    uint8_t  tf2_filepath_len;
       char *tf2_filepath;

    uint32_t player_records_len;
    struct player_record_0 *player_records;
};

extern bool save_format_1_save(const struct save_format_1 *save_data, FILE *output_file_pointer);

extern bool save_format_1_load(struct save_format_1 *save_data, FILE *input_file_pointer);

extern bool save_format_1_free(struct save_format_1 *save_data);

#endif // SAVE_FORMATS_VERSION_0_H
