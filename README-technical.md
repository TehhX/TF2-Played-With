# Technical README

Contains information for development and related endeavors.

## Building

TF2PW is best built via CMake. Only the standard CMake commands are required to build/run TF2PW, find CMake tutorials somewhere else. Installing via CMake is not yet implemented.

## Dependencies

The following are required for building:

* [GNU-C's argp](https://sourceware.org/glibc/)
  * For argument parsing via `argp.h`
  * Not sure of its Windows availability
* [Cider](https://github.com/TehhX/Cider)
  * For saving/loading

## Glossary

* STEAMID3 Excerpt (SID3E): If a user's STEAMID3 is `[U:1:{}]`, their STEAMID3 excerpt is `{}`. For example, my STEAMID3 is `[U:1:324394636]`, therefore my STEAMID3 excerpt is `324394636`.
* Various path words are taken from [Cider's glossary](https://github.com/TehhX/Cider/blob/main/include/cider.h#L12)

## Getting Players From TF2

TF2 Played With will run alongside TF2 and get output from the console via an intermediary file. In game, the command `con_logfile log.txt` will be used to log *all* console output. See [TF2 Useful Console Commands] for more info on this command.

### Console Output Notes

Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:

* Will only output `#` as first character in new line when printing player status
* Players whose names *start* with `#` will have it replaced with a space. Following occurrences of `#` will remain intact
* Player names can have `"`s anywhere in their name without any escape characters in the output of `status` to denote them as special
* The regex `^# {2,}` will output *only* lines output by `status` relating player names to associated data
* The regex `connected$` will output *only* lines notifying of player connect events, including the user's own
* The first instance of `<NAME> connected` will always contain the user's name
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

* Status output will sometimes not start with # userid ... as a batch
* Status output will sometimes be interrupted by seemingly unrelated output
* Names and STEAMID3's can be captured by the regex `"(.+)" +\[U:1:([0-9]+)\]`, where group-1 is name and group-2 is STEAMID3 excerpt

### When Status Should be Run

Status must be run at least once per game. However, running more than once will allow it to capture players who join after me. Because I personally have to run the status command, binding it to a commonly pressed key will run it much more than frequently enough to record every player, for example W, A, S, D or LMB.

If it is done this way, this software will generate an incredible excess of match dates and names per STEAMID3 depending on how much I press the designated button. To solve this, a single status output will be set as the first/ultimate instance per-match, and only record newcomers to the match afterward.

The master status-report will occur once after each initial join message in the form `<NAME> Connected`, eg. `TehX Connected`. After launching the game for the first time, the user's name will need to be taken via the first invocation of `status`.

## Save File

### Format Versions

Each save file will contain a version number of the formatting type and should be saved/loaded in the same way as that type. Until 1.0.0, the save format is always 0 and is subject to frequent and heavy modifications.

### Visualization

The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure). `{ ... }` means there is an undefined count of the above element in the array).

    {
        // File header, always "TF2PW"
        (char *) HEADER,

        // The version of the save format
        (u8) SAVE_VERSION,

        (struct player_record *) PLAYER_RECORDS:
        [
            {
                // STEAMID3 excerpt of the player whose records are in the following array
                (u32) USER_STEAMID3_EXCERPT,

                (struct date_record *) DATE_RECORDS:
                [
                    {
                        // Date in days since UNIX epoch
                        (u16) DATE_IN_DAYS,

                        // The encountered player's name on this day
                        (char *) NAME,

                        // Times encountered on this day
                        (u8) ENCOUNTER_COUNT
                    },

                    {...}
                ]
            },

            {...}
        ]
    }

### Considerations

* `player_record.name` assumes a player's name won't be changed during individual dates the user encounters them. Maybe an array of their names? Thoughts for another save format version in any case
* To change a name in TF2, the player must quit the game and relaunch it

### Structure

|         Name          |                                 Description                                 |                     Size (Bytes)                      |              Example               |
|:---------------------:|:---------------------------------------------------------------------------:|:-----------------------------------------------------:|:----------------------------------:|
|        Header         |       Header of the file format. Always "TF2PW", else file is invalid       |                           5                           |               TF2PW                |
|  Save Format Version  |      The version of history file format used with this particular file      |                           1                           |               (u8) 0               |
| Player Records Length |       How many unique player records there are in the following array       |                           4                           |            (u32) 12,000            |
| Date Records Lengths  |      An array of lengths for every date record in every player record       |               4 * player records length               |      (u32 *) { 4, 9, 13, 2 }       |
|     Name Lengths      | An array of lengths for each name in each date record in each player record | 1 * player records length * sum(date records lengths) |      (u8 *) { 13, 4, 8, 15 }       |
|   STEAMID3 Excerpts   |            An array of STEAMID3 excerpts for each player record             |               4 * player records length               |       (u32 *) { 22202, ... }       |
|         Dates         |         How many days since the epoch this date record was recorded         |             2 * sum(date records lengths)             | (u16 *) { 20,530, 20,622, 20,625 } |
|         Names         |                   An array of names for each date record                    |                 1 * sum(name lengths)                 |    (i8 *) { Rabscuttle, Robin }    |
|   Encounter Counts    |  An array of how many times a player was encountered for each date records  |                 1 * sum(name lengths)                 |       (u8 *) { 3, 2, 8, 19 }       |

## Todo

Technical things which should be worked out:

* Probably won't need user SID3E in history file whatsoever, recheck after program actually works
* Figure out argp for Windows
* Implement installing via CMake
* Create builds for Windows and Linux
* Implement history files
* See if there's a way to output commands directly to a file, console window gets easily clogged
* Consider splitting this README into end-user/technical README's
* The entire history file is written to in its entirety every save/load when only certain parts may require rewriting
* Address TODO comments in the source code
  * BSEARCH_TODO: Implement binary search algorithm and value sorting where this TODO is present
* The name is a bit clunky
