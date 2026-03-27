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

extern uint8_t history_initialized;

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
#define HISTORY_SET_DATE_TODAY ((uint16_t) 1)

// Convert UE-seconds to a date in days
#define UES_TO_DAYS(UES) ((UES) / SECONDS_PER_DAY)

// Convert date in days to UE-seconds
#define DAYS_TO_UES(DAYS) ((DAYS) * SECONDS_PER_DAY)

/*
	Sets the date variable to a new date
		@param new_date: A pointer to the new date to set. If equal to HISTORY_SET_DATE_TODAY, set date to today
*/
extern void history_set_date(uint16_t new_date);

/*
	Adds a player to the records
		@param p_info: Should point to a struct player_info containing data on the player to add/update a record for
*/
extern void history_add_record(const struct player_info *p_info);

/*
	Prints a record's data to given stream
		@param requested_sid3e: The STEAMID3 excerpt to retrieve
*/
extern void history_print_record(uint32_t requested_sid3e);

/*
	Prints all records of players which have ever had `name` as their username
		@param name: The username of the requested player
*/
extern void history_print_records(const char *name);

#endif // HISTORY_H
