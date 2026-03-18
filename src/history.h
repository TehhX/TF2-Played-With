#ifndef HISTORY_H
#define HISTORY_H

/*
    history.h
    ---------
    Contains functionality for saving and loading local history file.
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

#endif // HISTORY_H
