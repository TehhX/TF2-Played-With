# TF2 Played With
A repository to keep track of which players I've played with in Valve's Team Fortress 2. The name is a bit clunky, and thus subject to change.

## Functionality
The software will have the ability to do the following:
* Add [data](#saved-data) to history file
* Retreive data from history file by name or STEAMID3
* Print current player histories during gameplay

## Getting Players From TF2
TF2 Played With will run alongside TF2 and get output from the console.

### Console Output Notes
Analyzing console output from entire sessions has produced valuable information. Some information may later be found false or generally unreliable, so only trust the below list generally:
* Will only output `#` as first character in new line when printing player status. I suspect it will also occur if a player with `#` as the first character in their name types in a chat I have access to, eg. match chat

### Status Formatting
Running the `status` command will return a list of players in the server. It is usually formatted as so:
```
# userid name                uniqueid            connected ping loss state
#  <UID> "<NAME>"            [<STEAMID3>]        <VV:VV>    <V>  <V> <V>
... // More names and stuff in the same form
```

### When Status Should be Run
Status must be run at least once per game. However, running more than once will allow it to capture players who join after me. Because I personally have to run the status command, binding it to a commonly pressed key will run it much more than frequently enough to record every player, for example W, A, S, D or LMB. If it is done this way, this software will generate an incredible excess of match dates and names per STEAMID3 depending on how much I press the designated button. There are a couple ways I can think of to overcome this:
* Set a status command to be the first/ultimate instance per-match, only record newcomers to the match afterward. Then, when a condition has been fulfilled, re-record everyone when...
    * My connected time becomes less than last recorded.
    * The initial connection message (Connected to 169.254.XXX.XXX:XXXX) is output
    * After I echo something, for example bind f1 "echo PENULTIMATE_STATUS ; status"
    * The initial join message "\<NAME\> Connected" (eg. TehX Connected)

Chosen method:
    The initial join message "\<NAME\> Connected" (eg. TehX Connected)

Notes on status formatting:
* Will sometimes not start with # userid ...
* Will sometimes be interrupted by seemingly unrelated output
* Names and STEAMID3's can be captured by the regex `"(.+)" +\[U:([0-9]:[0-9]+)\]`, where group-1 is name and group-2 is STEAMID3.

## Save File
Data will need to be saved to track players, and will be stored in a central file.

### Required Data
The following data will be required (JSON format here for clarification, binary format actually used in program as described under [file structure](#structure)):
```
{
    "Name": "TehX",

    "Data":
    [
        {...},

        <STEAMID3>:
        [
                {
                    <DATE_IN_SECONDS>,
                    "<NAME>",
                    <ENCOUNTER_COUNT>
                },

                {...} // More dates and assocated data
        ],

        {...} // More STEAMID3's and associated data
    ]
}
```

### Structure
TODO: Create history file structure and put here

## Helpful Resources
Websites which could be useful in this repo:
* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
* [SteamID I/O](https://steamid.io/)
* [Source Console Useful Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands), in particular con_log \<file\>. I found the actual command to be con_logfile \<file\>.
