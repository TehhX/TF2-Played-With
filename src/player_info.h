#ifndef PLAYER_INFO_H
#define PLAYER_INFO_H

/*
    player_info.h
    -------------
    Contains definitions for relating a player's SID3E to their name. Has no accompanying .c file
*/

#include "stdint.h"

// Steam says max bytes in a name is 32, so I'm using 64 to be safe. Will be allocated on the stack
typedef char steam_name_stack[64];

// Pairs a player's name to a player's SID3E
struct player_info
{
	uint32_t sid3e;
	steam_name_stack name;
};

#endif // PLAYER_INFO_H
