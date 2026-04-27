# TF2 Played With

 A small command-line program to keep track of people you play with in Team Fortress Two. For a more technical rundown of the software itself or building instructions, see the [technical README](/README-technical.md). Run tf2pw --help for more.

## Download

Download pre-compiled binaries from [releases](https://github.com/TehhX/TF2-Played-With/releases/latest), or check the [building instructions](/README-technical.md#building) to build it yourself.

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
