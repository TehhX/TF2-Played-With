# TF2 Played With

A repository to keep track of which players I've played with in Valve's Team Fortress 2. For a more technical rundown of the software itself or building instructions, see the [technical README](/README-technical.md).

## Download

TF2PW is not yet viable so no builds are available, nor would I recommend you use them if they were.

<!-- Multi-Use Links: -->
[SourceCmd]: https://github.com/rannmann/SourceCmd
[ConsoleForwarder]: https://github.com/SNWCreations/ConsoleForwarder

## Functionality

The software will have the ability to do the following:

* Add [data](/README-technical.md#visualization) to history file during gameplay
* Retrieve data from history file by name, STEAMID64 or STEAMID3, display in a human-readable format
* Print histories of players encountered during gameplay
* Stop printing (but not stop collecting) during gameplay to accept user input via CLI. Input may include:
  * Getting data from history
  * Stop tf2pw
  * Save to disk
  * Load from disk

## Helpful Resources and Thanks

* [SteamID Wiki Page](https://developer.valvesoftware.com/wiki/SteamID)
* [TF2 Useful Console Commands](https://developer.valvesoftware.com/wiki/Developer_console#Useful_commands)
  * `con_logfile <file>`: Data output
  * `status`: Data retrieval
  * `echo <string>`: Taking notes mid-game
* [TehhX/Learning-C](https://github.com/TehhX/Learning-C): Various implementations from here
* [SourceCmd]: Might have some useful things in it, have to comb through it
* [ConsoleForwarder]: Might have similar use as SourceCmd, have to look at it some time
* [Team Fortress 2](https://store.steampowered.com/app/440)
  * The source of all console output for analysis and testing
  * A great game that this entire repo is made for

## Future Improvement Ideas

General ideas and brainstorming for new features:

* Other games (Same or different repo?)
* Requesting specific info on player instead of vomiting it all out
* Requesting info by date

Proposed save format changes:

|                                  Idea Description                                   | Expected Save Format Version Range |
|:-----------------------------------------------------------------------------------:|:----------------------------------:|
|                       Compress same names across date records                       |               [0,2]                |
|                The ability to record notes about players or servers                 |                 0                  |
|                 Record kills/deaths regarding user for each player                  |               [1,2]                |
| Record servers connected to along with relevant server records (K/D, maps seen etc) |             [1,never]              |
