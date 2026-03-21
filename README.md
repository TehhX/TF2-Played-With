# TF2 Played With
A repository to keep track of which players I've played with in Valve's Team Fortress 2. The name is a bit clunky, and thus subject to change.

## Download
TF2PW is not yet viable so no builds are available, nor would I reccomend you use them if they were. If you are a C programmer and/or want to mess around with what it currently does, check [building](#building). Just keep in mind this isn't even in alpha yet.

## Glossary
* STEAMID3 Excerpt: If a user's STEAMID3 is `[U:1:{}]`, their STEAMID3 excerpt is `{}`. For example, my STEAMID3 is `[U:1:324394636]`, therefore my STEAMID3 excerpt is `324394636`.
* Various path words are taken from [Cider's glossary](https://github.com/TehhX/Cider/blob/main/include/cider.h#L12)

<!-- Multi-Use Links: -->
[TF2 Useful Console Commands]: https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands

## Functionality
The software will have the ability to do the following:
* Add [data](#required-data-and-visualization) to history file during gameplay
* Retreive data from history file by name, STEAMID64 or STEAMID3, display in a human-readable format
* Print histories of players encountered during gameplay

## Dependencies
The following are required for building:
* [GNU-C's argp](https://sourceware.org/glibc/)
    * For argument parsing via `argp.h`
    * Not sure of its Windows availability
* [Cider](https://github.com/TehhX/Cider)
    * For saving/loading

## Getting Players From TF2
TF2 Played With will run alongside TF2 and get output from the console via an intermediary file. In game, the command `con_logfile log.txt` will be used to log *all* console output. See [TF2 Useful Console Commands] for more info on this command.

### Console Output Notes
Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:
* Will only output `#` as first character in new line when printing player status. I suspect it will also occur if a player with `#` as the first character in their name types in a chat I have access to, eg. match chat
* `<NAME> connected` will output when entering a map, while `Connected to <IP>` will output when connecting to a new server. Example events for clarification:
    * Queue for game, join server:
        * `Connected to <IP>`
        * `<NAME> connected`
    * Enter map based on vote in same server:
        * `<NAME> connected`
    * Requeuing in match:
        * `Connected to <IP>`
        * `<NAME> connected`
* Each match is only guaranteed to output `<NAME> connected`, and not necessarily `Connected to <IP>`.
* `<NAME> connected` may print erroneously if there is another player by the same name as the user

#### Status Formatting
Running the `status` command will return a list of players in the server. It is usually formatted as so:

    # userid name                uniqueid            connected ping loss state
    #  <UID> "<NAME>"            [<STEAMID3>]        <VV:VV>    <V>  <V> <V>
    ... // More names and stuff in the same form

#### Further observations
* Status output will sometimes not start with # userid ...
* Status output will sometimes be interrupted by seemingly unrelated output
* Names and STEAMID3's can be captured by the regex `"(.+)" +(\[U:1:[0-9]+\])`, where group-1 is name and group-2 is STEAMID3

### When Status Should be Run
Status must be run at least once per game. However, running more than once will allow it to capture players who join after me. Because I personally have to run the status command, binding it to a commonly pressed key will run it much more than frequently enough to record every player, for example W, A, S, D or LMB.

If it is done this way, this software will generate an incredible excess of match dates and names per STEAMID3 depending on how much I press the designated button. To solve this, a single status output will be set as the first/ultimate instance per-match, and only record newcomers to the match afterward.

The master status-report will occur once after each initial join message in the form `<NAME> Connected`, eg. `TehX Connected`. After launching the game for the first time, the user's name will need to be taken via the first invocation of `status`.

## Save File
### Save Format Versions
Each save file will contain a version number of the formatting type and should be saved/loaded in the same way as that type. Until 1.0.0, the save format is always 0 and is subject to frequent and heavy modifications.

### Required Data and Visualization
The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure). Note that below data will not be in the same order as the binary file. `{ ... }` means there is an undefined count of the above element in the array).

    tf2pwXX.sav:
    {
        // File header, always "TF2PW"
        (char *) HEADER

        // The version of the save format
        (u8) SAVE_VERSION

        // STEAMID32 excerpt of user (the person with this file on their computer)
        (u32) USER_STEAMID3_EXCERPT

        // An array of lengths for each PLAYER_RECORD
        (u32) DATE_RECORDS_LEN

        // How many unique player records there are in PLAYER_RECORDS
        (u32) PLAYER_RECORDS_LEN
        (struct player_record *) PLAYER_RECORDS:
        [
            {
                // STEAMID3 excerpt of the player whose records are in the following array
                (u32) USER_STEAMID3_EXCERPT

                // How many date_records there are in DATE_RECORDS
                (u32) DATE_RECORDS_LEN
                (struct date_record *) DATE_RECORDS:
                [
                    {
                        // Date in days since UNIX epoch
                        (u16) DATE_IN_DAYS

                        // The length of NAME
                        (u8) NAME_LEN
                        (char *) NAME

                        // Times encountered on this day
                        (u8) ENCOUNTER_COUNT
                    }

                    {...}
                ]
            }

            {...}
        ]
    }

### Considerations
* `player_record.name` assumes a player's name won't be changed during individual dates the user encounters them. Maybe an array of their names? Thoughts foor another save format version in any case

### Structure
##### Header
Always at the beginning of every save file.
|         Name          |                                 Description                                 |                     Size (Bytes)                      |              Example               |
|:---------------------:|:---------------------------------------------------------------------------:|:-----------------------------------------------------:|:----------------------------------:|
|        Header         |                          Header of the file format                          |                           5                           |           TF2PW (ALWAYS)           |
|  Save Format Version  |      The version of history file format used with this particular file      |                           1                           |               (u8) 0               |
| User STEAMID3 excerpt |                 The STEAMID3 excerpt of the recorder/user.                  |                           4                           |          (u32) 324394636           |
| Player Records Length |       How many unique player records there are in the following array       |                           4                           |            (u32) 12,000            |
| Date Records Lengths  |      An array of lengths for every date record in every player record       |               4 * player records length               |      (u32 *) { 4, 9, 13, 2 }       |
|     Name Lengths      | An array of lengths for each name in each date record in each player record | 1 * player records length * sum(date records lengths) |      (u8 *) { 13, 4, 8, 15 }       |
|   STEAMID3 Excerpt    |            An array of STEAMID3 excerpts for each player record             |               4 * player records length               |       (u32 *) { 22202, ... }       |
|         Dates         |         How many days since the epoch this date record was recorded         |             2 * sum(date records lengths)             | (u16 *) { 20,530, 20,622, 20,625 } |
|         Names         |                   An array of names for each date record                    |                 1 * sum(name lengths)                 |   (char *) { Rabscuttle, Robin }   |
|   Encounter Counts    |  An array of how many times a player was encountered for each date records  |                           1                           |       (u8 *) { 3, 2, 8, 19 }       |

## Building
TF2PW is best built via CMake. Only the standard CMake commands are required to build/run TF2PW, find CMake tutorials somewhere else. Installing via CMake is not yet implemented.

## Helpful Resources and Thanks
* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
    * Structure of STEAMID3
    * STEAMID is 8 bytes in size, multiple human-readable formats
* [SteamID I/O](https://steamid.io/)
* [SteamDB](https://steamdb.info/)
    * Thanks for the [ID conversion code](/meta/steamdb_id_conversion.js)
* [TF2 Useful Console Commands]
    * con_logfile \<file\>.
* [TehhX/Learning-C](https://github.com/TehhX/Learning-C)
    * Contains files I use for testing various concepts within this and other repositories

## Todo
* Implement installing via CMake
* Create builds for Windows and Linux
* Figure out argp for Windows
* Implement history files
