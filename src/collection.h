#ifndef COLLECTION_H
#define COLLECTION_H

/*
	collection.h
	-------------
	Contains functionality for collecting data into history memory from log files.
*/

#include "stdbool.h"
#include "stdio.h"

struct collection_read_live_routine_params
{
	FILE *input_file; // Input file handle
	bool running;     // True if running
};

// @warning Only use this directly if calling without thread, else use definition collection_read_live_routine
extern void *_collection_read_live_routine(struct collection_read_live_routine_params *params);

/*
	Begins collecting data live from `.../tf/log.txt`. If you have a complete log file without more changes coming, use collection_read_archived instead

		@param params: Pass struct `collection_read_live_routine_params`

		@note This is a definition typecast from _collection_read_live_routine for use with pthreads
*/
#define collection_read_live_routine ((void *(*)(void *)) _collection_read_live_routine)

/*
	Collects data from old/archive files which aren't currently being written to. Use collection_read_live for that

		@param collection_fullname: Should be a log file previously written to by TF2
*/
extern void collection_read_archived(const char *collection_fullname);

#endif // COLLECTION_H
