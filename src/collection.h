#ifndef COLLECTION_H
#define COLLECTION_H

/*
	collection.h
	-------------
	Contains functionality for collecting data into history memory from log files.
*/

#include "stdatomic.h"
#include "stdbool.h"
#include "stdio.h"

struct collection_read_live_routine_params
{
	atomic_bool continue_running;    // True if continue, else false
	FILE *input_file;                // Input file
};

/*
	Begins collecting data live from `.../tf/log.txt`. If you have a complete log file without more changes coming, use collection_read_archived instead
		@param params: Pass struct `collection_read_live_routine_params`
*/
extern void *collection_read_live_routine(void *params);

/*
	Collects data from old/archive files which aren't currently being written to. Use collection_read_live for that
		@param collection_fullname: Should be a log file previously written to by TF2
*/
extern void collection_read_archived(const char *collection_fullname);

#endif // COLLECTION_H
