#ifndef PLAYER_INFO_H
#define PLAYER_INFO_H

/*
    player_info.h
    -------------
    Contains definitions for relating a player's SID3E to their name. Has no accompanying .c file
*/

#include "stdint.h"

// Pairs a player's name to a player's SID3E
struct player_info
{
	uint32_t sid3e;
	char *name;
};

#endif // PLAYER_INFO_H
