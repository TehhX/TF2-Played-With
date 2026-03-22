#ifndef HISTORY_H
#define HISTORY_H

/*
	history.h
	---------
	Contains functionality for saving and loading local history file, as well as input/output for history.
*/

#include "common.h"

// 1 if initialized, else 0
extern int history_initialized;

/* Initialize history
	   @param history_fullname: The file to save/load. If NULL, use default fullname, else must be on the heap */
extern void history_init(char *history_fullname);

// Free resources used by history
extern void history_free();

// Loads history file ~/.local/share/tf2pwXX.sav into memory
extern void history_load();

// Saves memory into history file ~/.local/share/tf2pwXX.sav
extern void history_save();

/*
	Begins collecting data live from collections_fullname. If you have a complete log file without more changes coming, use history_collect_archived instead
		@param collections_fullname: Should be the TF2 console output file fullname currently being written to
*/
extern void history_collect_live(const char *collections_fullname);

/*
	Collects data from old/archive files which aren't currently being written to. Use history_collect_live for that
		@param collections_fullname: Should be a log file previously written to by TF2
*/
extern void history_collect_archived(const char *collections_fullname);

#endif // HISTORY_H
