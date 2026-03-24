#ifndef collection_H
#define collection_H

/*
	collection.h
	-------------
	Contains functionality for collecting data into history memory from log files.
*/

/*
	Begins collecting data live from collection_fullname. If you have a complete log file without more changes coming, use collection_read_archived instead
		@param collection_fullname: Should be the TF2 console output file fullname currently being written to
*/
extern void collection_read_live(const char *collection_fullname);

/*
	Collects data from old/archive files which aren't currently being written to. Use collection_read_live for that
		@param collection_fullname: Should be a log file previously written to by TF2
*/
extern void collection_read_archived(const char *collection_fullname);

#endif // collection_H
