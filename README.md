# TF2 Played With

A small command-line program to keep track of people you play with in Team Fortress Two.

## Download

Download pre-compiled binaries from the [latest release](https://github.com/TehhX/TF2-Played-With/releases/latest), or check the [building instructions](/README.md#building) to build it yourself.

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

## Stability

Any major/minor release (those with versions in the form `TF2PW vX.Y.0`) will be perfectly stable and usable, but major save/functionality-breaking changes are possible from patch to patch. To be completely sure of its stability, simply download one of TF2PW's [official releases](https://github.com/TehhX/TF2-Played-With/releases/).

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
* Player names can have `"`s anywhere in their name without `status` modifying them whatsoever
* The regex `^# {2,}` will output *only* lines output by `status` relating player names to associated data
* If last occurrence of string `BOT` is later than last occurrence of character `]`, player is a bot
* Players with duplicate names will have their names changed by the following algorithm (assuming no abuse of whitespace characters vis-a-vis the bot crisis, where seemingly duplicate names are actually):
  * 1: `<NAME>`
  * 2: (1)`<NAME>`
  * n {n > 1}: `(<n - 1>)<NAME>`
* `Client reached server_spawn.` will output when entering a new map/server
* On quit, seems to always print `CTFGCClientSystem::ShutdownGC`. Doesn't seem to print at other times. If multiple session outputs (launch to quit) are in the same file, it could also be used to differentiate and treat it as a new file
* Chat output can be captured by the following regex: `(\*DEAD\*){0,1}(\(TEAM\)){0,1}( ){0,1}((.+)) :  (.*)`, where group 5 is the player's name, and group 6 is the message

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

    // The TF2 filepath
    (string) TF2_FILEPATH,

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

                    // Times encountered on this day (minus one)
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

### Structure

#### Header

|          Name           |                                         Description                                         |                 Size (Bytes)                  |                                  Example                                   |
|:-----------------------:|:-------------------------------------------------------------------------------------------:|:---------------------------------------------:|:--------------------------------------------------------------------------:|
|         Header          |               Header of the file format. Always "TF2PW", else file is invalid               |                       5                       |                              (char *) "TF2PW"                              |
|   Save Format Version   |              The version of history file format used with this particular file              |                       1                       |                                   (u8) 0                                   |
|       User SID3E        |                              The STEAMID3 excerpt of the user                               |                       4                       |                               (u32) 12345678                               |
| Default Record Messages |         The default value for Player Record::Record Messages of new player records          |                       1                       |                                   (u8) 0                                   |
|  Player Records Length  |               How many unique player records there are in the following array               |                       4                       |                                (u32) 12,000                                |
|   TF2 Filepath Length   |                   How many characters are in the Team Fortress 2 filepath                   |                       1                       |                                  (u8) 33                                   |
|      TF2 Filepath       |                                The Team Fortress 2 filepath                                 |              TF2 Filepath Length              | (char *) "/home/Timmy/.local/share/Steam/steamapps/common/Team Fortress 2" |
|     Player Records      | An array of player records. See [Player Record](#player-record) for its particular contents | Player Records Length * sizeof(Player Record) |                                    N/A                                     |

#### Player Record

|        Name         |                                                        Description                                                         |               Size (Bytes)                |                 Example                 |
|:-------------------:|:--------------------------------------------------------------------------------------------------------------------------:|:-----------------------------------------:|:---------------------------------------:|
|  STEAMID3 Excerpt   |                                                    This player's SID3E                                                     |                     4                     |               (u32) 22202               |
|   Record Messages   | Flag to check chat messages. 1 for yes, 0 for no. Will inherit value of Header::Record Messages until individually changed |                     1                     |                 (u8) 1                  |
|        Notes        |                                  Notes taken by the user on this player. Null terminated                                   |                 Variable                  | (char *) "They were very nice to me.\0" |
| Date Records Length |                                           How many date records this player has                                            |                     4                     |                (u32) 13                 |
|    Date Records     |                   An array of date records. See [Date Record](#date-record) for its particular contents                    | Date Records Length * sizeof(Date Record) |                   N/A                   |

#### Date Record

|      Name       |                                                  Description                                                   | Size (Bytes) |               Example               |
|:---------------:|:--------------------------------------------------------------------------------------------------------------:|:------------:|:-----------------------------------:|
|      Date       |                                  How many days since UNIX epoch this date was                                  |      2       |                20537                |
| Encounter Count |                      How many times this player was encountered on this date (minus one)                       |      1       |               (u8) 4                |
|   Name Length   |                                        The length of the following name                                        |      1       |               (u8) 13               |
|      Name       |                                      The name of the player on this date                                       | Name_Length  |          (char *) "Timmy"           |
|    Messages     | Will only exist if Record Messages in Player Record is 1. Contains messages sent from this player on this date |   Variable   | (char *) "Wow!\nCool!\nGood job!\0" |

## Inline TODO's

A list of TODO prefixes found in the source code and their meanings:

* MAJOR_TODO: Major/program-breaking issue under the right circumstances/changes
* IMMED_TODO: Issue up for immediate remediation, program won't work a large portion of the time or at all if not addressed. Should only commit with one of these if work *must* be stopped
* IMPL_TODO: A function or similar is simply not implemented. The program won't work as expected in run under circumstance(s) where it is called
* NEWARGS_TODO: Signifies a function or line(s) which will need modifying on introduction of a new argument. Nothing necessarily needs to be done when present, more serves as a reminder than an actual TODO
* SAVE_FORMAT_TODO: Signifies something to be changed with the release of a new save format version

## Helpful Resources and Thanks

* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
* [TF2 Useful Console Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands)
  * `con_logfile <file>`: Data output
  * `status`: Data retrieval
  * `echo <string>`: Taking notes mid-game
* [TehhX/Learning-C](https://github.com/TehhX/Learning-C): Various implementations from here
* [Team Fortress 2](https://store.steampowered.com/app/440)
  * The source of all console output for analysis and testing
  * A great game that this entire repo is made for
* [Steam-ID-Converter](https://github.com/ElektroStudios/Steam-ID-Converter/): For STEAMID conversion implementation
* [cholera²](https://steamcommunity.com/profiles/76561199183240914): For trusting a random TF2 player and, in so doing, paving the way for a solution to [issue #8](https://github.com/TehhX/TF2-Played-With/issues/8)
