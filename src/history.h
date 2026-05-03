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

/*
	@brief Loads history file ~/.local/share/tf2pwXX.sav into memory

		@param history_fullname The fullname of the history file to load from. Leave NULL to use default location
*/
extern void history_load(const char *history_fullname);

/*
	@brief Saves memory into history file ~/.local/share/tf2pwXX.sav

		@param history_fullname The fullname of the history file to save to. Leave NULL to use default location
*/
extern void history_save(const char *history_fullname);

// A sentinel value for use with history_set_date(...)
#define HISTORY_SET_DATE_TODAY ((uint16_t) 1)

/*
	@brief Sets the date variable to a new date

		@param new_date: A pointer to the new date to set. If equal to HISTORY_SET_DATE_TODAY, set date to today
*/
extern void history_set_date(uint16_t new_date);

/*
	@brief Sets the live_log_location to a new fullname

		@param new_tf2_filepath The new filepath. Should be free-able ie. on the heap. Its length should be at most UINT8_MAX without the trailing slash. If no trailing slash is present, will append one. Should not be used after calling this function unless repointed to return value as with `realloc(...)`

		@returns NULL if failed, else potentially realloc'd pointer to `new_tf2_filepath`
*/
extern char *history_set_tf2_filepath(char *new_tf2_filepath);

/*
	@brief Retrieves the live_log_location fullname

		@returns A string containing the fullname of the live log file. Should not be modified
*/
extern const char *history_get_live_log_fullname();

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
