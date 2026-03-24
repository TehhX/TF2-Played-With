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

// Variables
extern   uint8_t save_format_version;
extern  uint32_t user_steamid3_excerpt;
extern  uint32_t player_records_len;
extern  uint16_t current_date;

// Arrays
extern  uint32_t *date_records_lens;  extern size_t date_records_lens_len;
extern   uint8_t *name_lens;          extern size_t name_lens_len;
extern  uint32_t *steam_id3_excerpts; extern size_t steam_id3_excerpts_len;
extern  uint16_t *dates;              extern size_t dates_len;
extern    int8_t *names;              extern size_t names_len;
extern   uint8_t *encounter_counts;   extern size_t encounter_counts_len;

// 1 if history_init(...) has been called, else 0 (including if history_free() was called after history_init(...))
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

/*
	Adds a player to the records
		@param player_id: The SID3E of the player to add
		@param player_name: The current name of the player to add
*/
extern void history_add_record(const struct player_info *p_info);

#endif // HISTORY_H
