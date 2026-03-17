# TF2 Played With
A repository to keep track of which players I've played with in Valve's Team Fortress 2. The name is a bit clunky, and thus subject to change.

## Functionality
The software will have the ability to do the following:
* Add [data](#required-data-and-visualization) to history file during gameplay
* Retreive data from history file by name, STEAMID64 or STEAMID3, display in a human-readable format
* Print histories of players encountered during gameplay

## Dependencies
The following are required for building:
* [GNU-C's argp](https://sourceware.org/glibc/): Came installed with Arch for me, might have to do something special for other OS's

## Getting Players From TF2
TF2 Played With will run alongside TF2 and get output from the console.

### Console Output Notes
Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:
* Will only output `#` as first character in new line when printing player status. I suspect it will also occur if a player with `#` as the first character in their name types in a chat I have access to, eg. match chat

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
The following data will be required (Quasi-JSON format here for visualization, binary format actually used in program as described under [structure](#structure)):

    {
        // Name of recorder
        (u8) NAME_LEN,
        (char *) NAME,

        // Data array
        (u64) DATA_LEN,
        DATA:
        [
            {...}, // More STEAMI64's and associated data

            (u64) STEAMID64:
            [
                    {
                        (u32) DATE_IN_SECONDS,
                        (u8) NAME_LEN,
                        (char *) NAME,
                        (u8) ENCOUNTER_COUNT
                    },

                    {...} // More dates and assocated data
            ],

            {...}, // More STEAMID64's and associated data
        ]
    }

### Structure
TODO: Create binary history file structure and put here

## Helpful Resources and Thanks
Websites which could be useful in this repo:
* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
    * Structure of STEAMID3
    * STEAMID is 8 bytes in size, multiple human-readable formats
    * Storing STEAMID's is easiest with STEAMID64. The `status` command, however, returns STEAMID3's. Thankfully, SteamDB had website [JS code for conversion](/meta/steamdb_id_conversion.js) that I nabbed and refactored/ported to C
* [SteamID I/O](https://steamid.io/)
    * Thanks for the [ID conversion code](/meta/steamdb_id_conversion.js)
* [Source Console Useful Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands)
    * con_logfile \<file\>.
