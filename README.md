# TF2 Played With

A small command-line program to keep track of people you play with in Team Fortress Two.

## Download

Download pre-compiled binaries from [releases](https://github.com/TehhX/TF2-Played-With/releases/latest), or check the [building instructions](/README.md#building) to build it yourself.

## Help/FAQ

### The executable doesn't do anything when I start it

As TF2PW is a command-line-interface **only** application, make sure you're running it from a terminal, eg. command prompt on Windows, or any terminal emulator on Linux.

### Editing player record notes isn't working in some way

**Handy flowchart:**

* I don't know what a CLI text-editor is, don't have one installed, or both
  1. Look online for what a CLI text-editor is
  2. Choose and install one you like
  3. You now know what a CLI text-editor is and have one installed, return to the start of this flowchart and proceed accordingly
* I know what a CLI text-editor is ***and*** have one installed
  1. Set environment variable `EDITOR` to the name of your chosen editor. If you don't want to do this every time you launch TF2PW, you'll have to find how to set a permanent environment variable on your particular operating-system
  2. Make sure your editor is accessible via your `PATH` environment variable

**TL;DR:**

Make sure you have a CLI text-editor installed, that you've set the `EDITOR` environment variable to your editor's name, and that your editor is accessible via the `PATH` environment variable.

### The FAQ didn't help solve my particular problem

After making sure to re-read all FAQ items **thoroughly** (even those you don't believe have relevance), take one of the following actions:

* Submit a GitHub issue on this repository, making sure to use the `question` label
* Send me an email at [samuel.tobias@tehx.ca](mailto:samuel.tobias@tehx.ca) with `TF2PW Support` at the start of the subject line, eg. `TF2PW Support | Starting TF2PW`
* Scream into the wind

## Building

TF2PW is best built via CMake. Example:

```bash
cd TF2-Played-With
cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build build --config Release
```

## Dependencies

The following are required for building:

* [Cider](https://github.com/TehhX/Cider)
* pthreads
  * Comes with most Linux distros, but on windows you'll need [pthread-win32](https://github.com/GerHobbelt/pthread-win32)

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
* If last occurrence of string `BOT` is later than last occurrence of character `]`, player is a bot
* Players with duplicate names will have their names changed by the following algorithm (assuming no abuse of whitespace characters vis-a-vis the bot crisis, where seemingly duplicate names are actually):
  * 1: `<NAME>`
  * 2: (1)`<NAME>`
  * n {n > 1}: `(<n - 1>)<NAME>`
* Note that there is no space between the name and duplicate number/parentheses
* `<NAME> connected` and `Client reached server_spawn.` will output when entering a new map/server, while `Connected to <IP>` will output when connecting to a new server. Example events for clarification:
  * Queue for game, join server:
    * `Client reached server_spawn.`
    * `Connected to <IP>`
    * `<NAME> connected`
  * Enter map based on vote in same server:
    * `Client reached server_spawn.`
    * `<NAME> connected`
  * Requeuing in match:
    * `Client reached server_spawn.`
    * `Connected to <IP>`
    * `<NAME> connected`
* Each match is only guaranteed to output both `Client reached server_spawn.` and `<NAME> connected`, and not necessarily `Connected to <IP>`. For this reason, `Client reached server_spawn.` will be used
* On quit, seems to always print `CTFGCClientSystem::ShutdownGC`. Doesn't seem to print at other times. If multiple session outputs (launch to quit) are in the same file, it could also be used to differentiate and treat it as a new file
* General duplicated name observations:
  * User joins game first
    * `<NAME> connected`: Just like normal
    * `(1)<NAME> connected`: Other person, won't interfere as it's treated as a different name
  * User joins game second
    * `(1)<NAME> connected`: Won't output token for new match properly, is seen as another person
  * Solutions:
    * Instead of checking for `<NAME> connected`, check for (regex for example) `(\([0-9]{1,2}\))*<NAME> connected`
      * Pros:
        * Easy to implement
        * Will record new game in all required instances
      * Cons:
        * Will record new game when other player with user's name joins in any scenario
    * Instead of checking for `<NAME> connected`, check for `Client reached server_spawn.`. After light testing, it seems to print at the same times as `<NAME> connected`, only outputting more if player disconnects while in the loading screen. This causes no practical changes however, because the status array will be of length 0
      * Pros:
        * Easy to implement
        * User's name saving no longer required
        * Simpler checking
        * Duplicate names no longer have any problem
      * Cons:
        * None(?)
* Chat output can be captured by the following regex: `(\*DEAD\*){0,1}(\(TEAM\)){0,1}( ){0,1}((.+)) :  (.*)`, where group 5 is player name, and group 6 is message

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

Each save file will contain a version number of the formatting type and should be saved/loaded in the same way as that type.

### Visualization

The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure). `{ ... }` means there is an undefined count of the above element in the array).

```txt
{
    // File header, always "TF2PW"
    (string) HEADER,

    // The version of the save format
    (u8) SAVE_VERSION,

    // The STEAMID3 excerpt of the user
    (u32) USER_SID3E,

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

                    // Chat messages
                    (string[]) CHAT_MESSAGES,

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

* To change a name in TF2, the player must quit the game and relaunch it
* It is currently assumed that STEAMID3 excerpts will always be in the range [1, UINT32_MAX - 2] (for steamid_manip.h::SIDM_ERR_NONE_MAX reasons). It has held true for now, but it is still only an assumption made for space savings

### Structure

#### Header

|          Name           |                                         Description                                         |     Size (Bytes)      |                            Example                            |
|:-----------------------:|:-------------------------------------------------------------------------------------------:|:---------------------:|:-------------------------------------------------------------:|
|         Header          |               Header of the file format. Always "TF2PW", else file is invalid               |           5           |                             TF2PW                             |
|   Save Format Version   |              The version of history file format used with this particular file              |           1           |                            (u8) 0                             |
|       User SID3E        |                              The STEAMID3 excerpt of the user                               |           4           |                        (u32) 12345678                         |
| Default Record Messages |         The default value for Player Record::Record Messages of new player records          |           1           |                             (u8)0                             |
|    Live Log Path Len    |                            The length in bytes of Live Log Path                             |           1           |                            (u8) 82                            |
|      Live Log Path      |     The path to the live logging file. Should be the same as set in TF2 via con_logfile     | 1 * Live_Log_Path_Len | (char *) "/Steam/steamapps/common/Team Fortress 2/tf/log.txt" |
|  Player Records Length  |               How many unique player records there are in the following array               |           4           |                         (u32) 12,000                          |
|     Player Records      | An array of player records. See [Player Record](#player-record) for its particular contents |          N/A          |                              N/A                              |

#### Player Record

|        Name         |                                                        Description                                                         | Size (Bytes) |                 Example                 |
|:-------------------:|:--------------------------------------------------------------------------------------------------------------------------:|:------------:|:---------------------------------------:|
|  STEAMID3 Excerpt   |                                                    This player's SID3E                                                     |      4       |               (u32) 22202               |
|   Record Messages   | Flag to check chat messages. 1 for yes, 0 for no. Will inherit value of Header::Record Messages until individually changed |      1       |                 (u8) 1                  |
|        Notes        |                                  Notes taken by the user on this player. Null terminated                                   |     N/A      | (char *) "They were very nice to me.\0" |
| Date Records Length |                                           How many date records this player has                                            |      4       |                (u32) 13                 |
|    Date Records     |                   An array of date records. See [Date Record](#date-record) for its particular contents                    |     N/A      |                   N/A                   |

#### Date Record

|      Name       |                                                  Description                                                   |  Size (Bytes)   |               Example               |
|:---------------:|:--------------------------------------------------------------------------------------------------------------:|:---------------:|:-----------------------------------:|
|      Date       |                                  How many days since UNIX epoch this date was                                  |        2        |                20537                |
| Encounter Count |                            How many times this player was encountered on this date                             |        1        |               (u8) 4                |
|   Name Length   |                                        The length of the following name                                        |        1        |               (u8) 13               |
|      Name       |                                      The name of the player on this date                                       | 1 * Name_Length |           (i8 *) "Timmy"            |
|    Messages     | Will only exist if Record Messages in Player Record is 1. Contains messages sent from this player on this date |       N/A       | (char *) "Wow!\nCool!\nGood job!\0" |

## Inline TODO's

A list of TODO prefixes found in the source code and their meanings:

* MAJOR_TODO: Major/program-breaking issue under the right circumstances/changes
* IMMED_TODO: Issue up for immediate remediation, program won't work a large portion of the time or at all if not addressed. Should only commit with one of these if work *must* be stopped
* IMPL_TODO: A function or similar is simply not implemented. The program won't work as expected in run under circumstance(s) where it is called
* NEWARGS_TODO: Signifies a function or line(s) which will need modifying on introduction of a new argument. Nothing necessarily needs to be done when present, more serves as a reminder than an actual TODO

## Helpful Resources and Thanks

* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
* [TF2 Useful Console Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands)
  * `con_logfile <file>`: Data output
  * `status`: Data retrieval
  * `echo <string>`: Taking notes mid-game
* [TehhX/Learning-C](https://github.com/TehhX/Learning-C): Various implementations from here
* [SourceCmd](https://github.com/rannmann/SourceCmd): Might have some useful things in it, have to comb through it
* [ConsoleForwarder](https://github.com/SNWCreations/ConsoleForwarder): Might have similar use as SourceCmd, have to look at it some time
* [Team Fortress 2](https://store.steampowered.com/app/440)
  * The source of all console output for analysis and testing
  * A great game that this entire repo is made for
* [Steam-ID-Converter](https://github.com/ElektroStudios/Steam-ID-Converter/): For STEAMID conversion implementation
* [cholera²](https://steamcommunity.com/profiles/76561199183240914): For trusting a random TF2 player and, in so doing, paving the way for a solution to [issue #8](https://github.com/TehhX/TF2-Played-With/issues/8)
