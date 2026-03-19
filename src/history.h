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
       @param file_num: The file to save/load in range [0, 99] */
extern void history_init(int file_num);

// Free resources used by history
extern void history_free();

// Loads history file ~/.local/share/tf2pwXX.sav into memory
extern void history_load();

// Saves memory into history file ~/.local/share/tf2pwXX.sav
extern void history_save();

/* Begins collecting data live from collections_fullname
        @param collections_fullname: Should be the TF2 console output file currently being written to. The file fullname from which to collect data */
extern void history_collect_live(const char *collections_fullname);

/* Collects data from old/archive files which aren't currently being written to. Use history_collect_live for that
        @param collections_fullname: Should be the TF2 console output file currently being written to. The file fullname from which to collect data */
extern void history_collect_archived(const char *collections_fullname);

#endif // HISTORY_H
