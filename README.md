# TF2 Played With
A repository to keep track of which players I've played with in Valve's Team Fortress 2. The name is a bit clunky, and thus subject to change.

## Functionality
The software will have the ability to do the following:
* Add [data](#required-data-and-visualization) to history file during gameplay
* Retreive data from history file by name, STEAMID64 or STEAMID3, display in a human-readable format
* Print histories of players encountered during gameplay

## Dependencies
The following are required for building:
* [GNU-C's argp](https://sourceware.org/glibc/)
    * For argument parsing
    * Not sure of its Windows compatability
* [Cider](https://github.com/TehhX/Cider)
    * For saving/loading

## Getting Players From TF2
TF2 Played With will run alongside TF2 and get output from the console via an intermediary file.

### Console Output Notes
Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:
* Will only output `#` as first character in new line when printing player status. I suspect it will also occur if a player with `#` as the first character in their name types in a chat I have access to, eg. match chat
* `<NAME> connected` will output when actually joining the server, while `Connected to <IP>` will output when connecting to a server.

#### Status Formatting
Running the `status` command will return a list of players in the server. It is usually formatted as so:

    # userid name                uniqueid            connected ping loss state
    #  <UID> "<NAME>"            [<STEAMID3>]        <VV:VV>    <V>  <V> <V>
    ... // More names and stuff in the same form

#### Further observations
* Status output will sometimes not start with # userid ...
* Status output will sometimes be interrupted by seemingly unrelated output
* Names and STEAMID3's can be captured by the regex `"(.+)" +\[U:([0-9]:[0-9]+)\]`, where group-1 is name and group-2 is STEAMID3

### When Status Should be Run
Status must be run at least once per game. However, running more than once will allow it to capture players who join after me. Because I personally have to run the status command, binding it to a commonly pressed key will run it much more than frequently enough to record every player, for example W, A, S, D or LMB.

If it is done this way, this software will generate an incredible excess of match dates and names per STEAMID3 depending on how much I press the designated button. To solve this, a single status output will be set as the first/ultimate instance per-match, and only record newcomers to the match afterward.

The penultimate status-report will occur once after each initial join message in the form `\n<NAME> Connected\n`, eg. `\nTehX Connected\n`. The '\n' characters

## Save File
### Required Data and Visualization
The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure). `{ ... }` means there is an undefined count of the above element in the array):

    tf2pwXX.sav:
    {
        // File header, always "TF2PW"
        (char *) HEADER

        // The version of the save format
        (u8) SAVE_VERSION

        // Name of recorder
        (u8) NAME_LEN
        (char *) NAME

        // How many unique player records there are in the following array
        (u32) PLAYER_RECORDS_LEN
        (struct []) PLAYER_RECORDS:
        [
            {
                // STEAMID64 of the player whose records are in the following array
                (u64) STEAMID64

                // How many unique dates they were encountered
                (u32) DATE_RECORDS_LEN
                (struct []) DATE_RECORDS:
                [
                    {
                        // Date in days since UNIX epoch
                        (u16) DATE_IN_DAYS

                        // TODO: Assumes a player's name won't be changed during dates user encounters them. Maybe an array of their names? For another version
                        // Name during this date's encounters
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

### Structure
<!-- TODO: Could be a bit cleaner, mostly using the above visualization for development because of that. Maybe standardize to other structure docs for other mainstream file types -->
#### Header
Always at the beginning of every save file.
|         Name         |                            Description                            | Size (Bytes) |    Example     |
|:--------------------:|:-----------------------------------------------------------------:|:------------:|:--------------:|
|        Header        |                     Header of the file format                     |      5       | TF2PW (ALWAYS) |
| Save Format Version  | The version of history file format used with this particular file |      1       |     (u8) 0     |
|     Name Length      |              The length (bytes) of user's Steam name              |      1       |     (u8) 4     |
|         Name         |                    The Steam name of the user                     | Name Length  |      TehX      |
| Player Record Length |  How many unique player records there are in the following array  |      4       |  (u32) 12,000  |

#### Unique Player Record
There will be one of these for every unique player record in the player record array.
|        Name         |                                  Description                                  | Size (Bytes) |         Example         |
|:-------------------:|:-----------------------------------------------------------------------------:|:------------:|:-----------------------:|
|      STEAMID64      |                  The STEAMID64 of the current player record                   |      8       | (u64) 76561197960287930 |
| Date Records Length | How many unique date records there are for this player in the following array |      4       |         (u32)38         |

#### Unique Player Date Record
There will be one of these for every unique player record's unique dates.
|      Name       |                       Description                       | Size (Bytes) |   Example    |
|:---------------:|:-------------------------------------------------------:|:------------:|:------------:|
|      Date       |       How many days since the epoch this date is        |      2       | (u16) 20,530 |
|   Name Length   |      The length of the player's name on this date       |      1       |   (u8) 10    |
|      Name       |           The name of the player on this date           | Name Length  |  Rabscuttle  |
| Encounter Count | How many times this player was encountered on this date |      1       |    (u8) 3    |

## Helpful Resources and Thanks
* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
    * Structure of STEAMID3
    * STEAMID is 8 bytes in size, multiple human-readable formats
    * Storing STEAMID's is easiest with STEAMID64. The `status` command, however, returns STEAMID3's. Thankfully, SteamDB had website [JS code for conversion](/meta/steamdb_id_conversion.js) that I nabbed and ported to C
* [SteamID I/O](https://steamid.io/)
    * Thanks for the [ID conversion code](/meta/steamdb_id_conversion.js)
* [Source Console Useful Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands)
    * con_logfile \<file\>.
* [TehhX/Learning-C](https://github.com/TehhX/Learning-C)
    * Contains files I use for testing various concepts within this and other repositories
