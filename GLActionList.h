/***********************************************************************

File: GLActionList.h
Author: Gregory Libera

Description: ActionList simply contains the ActionType enum. ActionType
is a list of all possible abilities that the user can preform. It is
placed into a seperate document as multiple class use ActionType and
doing so prevents conflicts.

***********************************************************************/

#ifndef __ACTIONLIST_H__
#define __ACTIONLIST_H__

namespace GAME
{
	enum ActionType
	{
		DEFAULT_ACTION,
		FIREBALL_ACTION,
		SLOW_ACTION,
		BUFF_ACTION,
		SPAWN_ACTION,
		TELEPORT_ACTION,
		CONFUSE_ACTION,
		STEALTH_ACTION,
		FIREWALL_ACTION,
	};
}
#endif