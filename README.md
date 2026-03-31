# TF2 Played With

A program to keep track of who you've played with in Valve's Team Fortress 2. For a more technical rundown of the software itself or building instructions, see the [technical README](/README-technical.md). Run tf2pw --help for more.

## Download

TF2PW is not yet viable so no builds are available, nor would I recommend you use them if they were.

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

## Future Improvement Ideas

General ideas and brainstorming for new features:

* Set history file live-log path via command line argument
* Requesting specific info on player instead of vomiting it all out
  * Date histories (current vomit)
  * First encounter date
  * Data for specific date
  * Last encountered
  * All names
  * Times name was changed
  * Most common name
  * Open profile in browser
* Requesting info by date
* Filters for retrieval eg. after/before date
* Retrieval wildcards/regex, especially for retrieving names
* Interactive mode auto-complete and up/down arrows for recent commands
* Option to record chat messages

Proposed save format changes:

|                                  Idea Description                                   | Expected Save Format Version Range |
|:-----------------------------------------------------------------------------------:|:----------------------------------:|
|                  Sorting for use of binary searches (BSEARCH_TODO)                  |                 0                  |
|                The ability to record notes about players or servers                 |                 0                  |
|                 Record kills/deaths regarding user for each player                  |               [1,2]                |
| Record servers connected to along with relevant server records (K/D, maps seen etc) |             [1,never]              |
