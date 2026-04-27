# TF2 Played With

 A small command-line program to keep track of people you play with in Team Fortress Two. For a more technical rundown of the software itself or building instructions, see the [technical README](/README-technical.md). Run tf2pw --help for more.

## Download

Check the [1.0.0 milestone](https://github.com/TehhX/TF2-Played-With/milestone/1) for when builds will be available for general download and use.

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

## FAQ

### The executable doesn't do anything when I start it

As TF2PW is a command-line-interface **only** application, make sure you're running it from a terminal, eg. command prompt on Windows, or any terminal emulator on Linux.
