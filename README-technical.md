# Technical README

Contains information for development and related endeavors.

## Building

TF2PW is best built via CMake. Only the standard CMake commands are required to build/run TF2PW, find CMake tutorials somewhere else. Installing via CMake is not yet implemented.

## Dependencies

The following are required for building:

* [GNU-C's argp](https://sourceware.org/glibc/)
  * For argument parsing via `argp.h`
  * Currently locking TF2PW to Linux, will be phased out
* [Cider](https://github.com/TehhX/Cider)
  * For saving/loading
* pthreads
  * For multithreading
  * Comes with most Linux distros, but on windows you'll need [pthread-win32](https://github.com/GerHobbelt/pthread-win32) (I haven't tested it well yet)

Headers and libraries can be put in /include/ and /lib/ respectively if your include/lib paths don't have any specific dependency.

## Glossary

* STEAMID3 Excerpt (SID3E): If a user's STEAMID3 is `[U:1:{}]`, their STEAMID3 excerpt is `{}`. For example, my STEAMID3 is `[U:1:324394636]`, therefore my STEAMID3 excerpt is `324394636`.
* Various path words are taken from [Cider's glossary](https://github.com/TehhX/Cider/blob/main/include/cider.h#L12)

## Getting Players From TF2

TF2 Played With will run alongside TF2 and get output from the console via an intermediary file. In game, the command `con_logfile log.txt` will be used to log *all* console output. See [TF2 Useful Console Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands) for more info on this command.

### Console Output Notes

Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:

* Will only output `#` as first character in new line when printing player status
* Players whose names *start* with `#` will have it replaced with a space. Following occurrences of `#` will remain intact
* Player names can have `"`s anywhere in their name without any escape characters in the output of `status` to denote them as special
* The regex `^# {2,}` will output *only* lines output by `status` relating player names to associated data
* The regex `connected$` will output *only* lines notifying of player connect events, including the user's own
* The first instance of `<NAME> connected` will always contain the TF2PW user's name
* If last occurrence of string BOT is later than last occurrence of character `]`, player is a bot
* Players with duplicate names will have their names changed by the following algorithm (assuming no abuse of whitespace characters vis-a-vis the bot crisis, where seemingly duplicate names are actually):
  * 1: `<NAME>`
  * 2: (1)`<NAME>`
  * n {n > 1}: `(<n - 1>)<NAME>`
* Note that there is no space between the name and duplicate number/parentheses
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

#### Status Formatting

Running the `status` command will return a list of players in the server. It is usually formatted as so (Lines starting with // are inserted by myself to denote things):

```txt
# userid name                uniqueid            connected ping loss state
// A real player
#  <UID> "<NAME>"            [<STEAMID3>]        <VV:VV>    <V>  <V> <V>
// A bot
#  <UID> "<NAME>"            BOT                                    active
// More output in the same form
```

### When Status Should be Run

Status must be run at least once per game. However, running more than once will allow it to capture players who join after me. Because I personally have to run the status command, binding it to a commonly pressed key will run it much more than frequently enough to record every player, for example W, A, S, D or LMB.

If it is done this way, this software will generate an incredible excess of match dates and names per STEAMID3 depending on how much I press the designated button. To solve this, a single status output will be set as the first/ultimate instance per-match, and only record newcomers to the match afterward.

The master status-report will occur once after each initial join message in the form `<NAME> Connected`, eg. `TehX Connected`. After launching the game for the first time, the user's name will need to be taken via the first invocation of `status`.

## Save File

### Format Versions

Each save file will contain a version number of the formatting type and should be saved/loaded in the same way as that type. Until 1.0.0, the save format is always 0 and is subject to frequent and heavy modifications.

### Visualization

The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure). `{ ... }` means there is an undefined count of the above element in the array).

```txt
{
    // File header, always "TF2PW"
    (string) HEADER,

    // The version of the save format
    (u8) SAVE_VERSION,

    // The path to the live logging file
    (string) LIVE_LOG_PATH,

    PLAYER_RECORDS:
    [
        {
            // STEAMID3 excerpt of the player whose records are in the following array
            (u32) STEAMID3_EXCERPT,

            (string) NOTES,

            DATE_RECORDS:
            [
                {
                    // Date in days since UNIX epoch
                    (u16) DATE,

                    // Times encountered on this day
                    (u8) ENCOUNTER_COUNT

                    // The encountered player's name on this date
                    (string) NAME
                },

                {...}
            ]
        },

        {...}
    ]
}
```

### Considerations

* `player_record.name` assumes a player's name won't be changed during individual dates the user encounters them. Maybe an array of their names? Thoughts for another save format version in any case
* To change a name in TF2, the player must quit the game and relaunch it
* It is currently assumed that STEAMID3 excerpts will always be in the range [1, UINT32_MAX - 2] (for steamid_manip.h::SIDM_ERR_NONE_MAX reasons). It has held true for now, but it is still only an assumption made for space savings

### Structure

#### Header

|         Name          |                                         Description                                         |     Size (Bytes)      |                            Example                            |
|:---------------------:|:-------------------------------------------------------------------------------------------:|:---------------------:|:-------------------------------------------------------------:|
|        Header         |               Header of the file format. Always "TF2PW", else file is invalid               |           5           |                             TF2PW                             |
|  Save Format Version  |              The version of history file format used with this particular file              |           1           |                            (u8) 0                             |
|   Live Log Path Len   |                            The length in bytes of Live Log Path                             |           1           |                            (u8) 82                            |
|     Live Log Path     |     The path to the live logging file. Should be the same as set in TF2 via con_logfile     | 1 * Live_Log_Path_Len | (char *) "/Steam/steamapps/common/Team Fortress 2/tf/log.txt" |
| Player Records Length |               How many unique player records there are in the following array               |           4           |                         (u32) 12,000                          |
|    Player Records     | An array of player records. See [Player Record](#player-record) for its particular contents |          N/A          |                              N/A                              |

#### Player Record

|        Name         |                                      Description                                      | Size (Bytes) |                 Example                 |
|:-------------------:|:-------------------------------------------------------------------------------------:|:------------:|:---------------------------------------:|
|  STEAMID3 Excerpt   |                                  This player's SID3E                                  |      4       |               (u32) 22202               |
|        Notes        |                Notes taken by the user on this player. Null terminated                |     N/A      | (char *) "They were very nice to me.\0" |
| Date Records Length |                         How many date records this player has                         |      4       |                (u32) 13                 |
|    Date Records     | An array of date records. See [Date Record](#date-record) for its particular contents |     N/A      |                   N/A                   |

#### Date Record

|      Name       |                       Description                       |  Size (Bytes)   |    Example     |
|:---------------:|:-------------------------------------------------------:|:---------------:|:--------------:|
|      Date       |      How many days since UNIX epoch this date was       |        2        |     20537      |
| Encounter Count | How many times this player was encountered on this date |        1        |     (u8) 4     |
|   Name Length   |            The length of the following name             |        1        |    (u8) 13     |
|      Name       |           The name of the player on this date           | 1 * Name_Length | (i8 *) "Timmy" |

## Inline TODO's

A list of TODO prefixes found in the source code and their meanings:

* MAJOR_TODO: Major/program-breaking issue under the right circumstances/changes
* IMMED_TODO: Issue up for immediate remediation, program won't work a large portion of the time or at all if not addressed. Should only commit with one of these if work *must* be stopped
* IMPL_TODO: A function or similar is simply not implemented. The program won't work as expected in run under circumstance(s) where it is called
* NEWARGS_TODO: Signifies a function or line(s) which will need modifying on introduction of a new argument. Nothing necessarily needs to be done when present, more serves as a reminder than an actual TODO
