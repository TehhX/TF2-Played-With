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
#include "stdbool.h"

/*
	@brief Initialize history

	   @param history_fullname: The file to save/load. If NULL, use default fullname, else must be on the heap
*/
extern void history_init(char *history_fullname);

/*
	@brief Check if history has been initialized (history_init(...) has been called)

		@returns A boolean containing initialization status
*/
extern bool history_is_initialized();

/*
	@brief Set user's STEAMID3 excerpt

		@param user_sid3e What to set the end-user's STEAMID3 excerpt to
*/
extern void history_set_user_sid3e(uint32_t user_sid3e);

/*
	@brief Get user's STEAMID3 excerpt

		@returns The end-user's STEAMID3 excerpt
*/
extern uint32_t history_get_user_sid3e();

// @brief Free resources used by history
extern void history_free();

// @brief Loads history file ~/.local/share/tf2pwXX.sav into memory
extern void history_load();

// @brief Saves memory into history file ~/.local/share/tf2pwXX.sav
extern void history_save();

// A sentinel value for use with history_set_date(...)
#define HISTORY_SET_DATE_TODAY ((uint16_t) 1)

/*
	@brief Sets the date variable to a new date

		@param new_date: A pointer to the new date to set. If equal to HISTORY_SET_DATE_TODAY, set date to today
*/
extern void history_set_date(uint16_t new_date);

/*
	@brief Sets the live_log_location to a new fullname

		@param live_log_location The new location. Should be free-able ie. on the heap

		@warning Triple check parameters for heapness(?)
*/
extern void history_set_live_log_location(char *live_log_location);

/*
	@brief Retrieves the live_log_location fullname

		@returns A constant pointer to the live_log_location
*/
extern const char *history_get_live_log_location();

/*
	@brief Adds a player to the records

		@param p_info: Should point to a struct player_info containing data on the player to add/update a record for
*/
extern void history_add_record(const struct player_info *p_info);

/*
	@brief Prints a record's data to given stream

		@param requested_sid3e: The STEAMID3 excerpt to retrieve
*/
extern void history_print_record(uint32_t requested_sid3e);

/*
	@brief Prints all records of players which have ever had `name` as their username

		@param name: The username of the requested player
*/
extern void history_print_records(const char *name);

/*
	@brief Opens up the user's editor to edit the notes of the requested player

		@param requested_sid3e The player's SID3E to edit

		@returns True if failure, else false

		@warning If user environment variable EDITOR is not set, this will use "vi" instead
		@warning Only tested with vim
*/
extern void history_edit_notes(uint32_t requested_sid3e);

/*
	@brief Adds a message to the current date record of player `requested_sid3e`

		@param requested_sid3e The STEAMID3 excerpt of the player for which to add the message
		@param message The message to add. Should be newline-terminated for the sake of collection.c
*/
extern void history_add_message(uint32_t requested_sid3e, const char *message);

#endif // HISTORY_H
