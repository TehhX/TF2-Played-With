#ifndef HISTORY_H
#define HISTORY_H

/*
	history.h
	---------
	Contains functionality for saving and loading local history file, as well as input/output for history.
*/

#include "player_info.h"

#include "stdint.h"
#include "stdlib.h"

extern  uint8_t history_initialized;
extern uint16_t current_date;

/*
	Initialize history
	   @param history_fullname: The file to save/load. If NULL, use default fullname, else must be on the heap
*/
extern void history_init(char *history_fullname);

// Free resources used by history
extern void history_free();

// Loads history file ~/.local/share/tf2pwXX.sav into memory
extern void history_load();

// Saves memory into history file ~/.local/share/tf2pwXX.sav
extern void history_save();

// A sentinel value for use with history_set_date(...)
#define HISTORY_SET_DATE_TODAY 0
/*
	Sets the date variable to a new date
		@param new_date: A pointer to the new date to set. If equal to HISTORY_SET_DATE_TODAY, set date to today
*/
extern void history_set_date(uint16_t new_date);

/*
	Adds a player to the records
		@param player_id: The SID3E of the player to add
		@param player_name: The current name of the player to add
*/
extern void history_add_record(const struct player_info *p_info);

#endif // HISTORY_H
