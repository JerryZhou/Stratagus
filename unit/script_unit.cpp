//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_unit.c	-	The unit ccl functions. */
//
//	(c) Copyright 2001-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ccl.h"
#include "spells.h"
#include "pathfinder.h"
#include "trigger.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Get unit-type.
#if defined(USE_GUILE) || defined(USE_SIOD)
extern UnitType* CclGetUnitType(SCM ptr);
    /// Get resource by name
extern unsigned CclGetResourceByName(SCM ptr);
#elif defined(USE_LUA)
    /// Get resource by name
extern unsigned CclGetResourceByName(lua_State* l);
#endif

/**
**	Set xp damage
**
**	@param flag Flag enabling or disabling it.
**
**	@return     The old state of the xp damage
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetXpDamage(SCM flag)
{
    int old;

    old = XpDamage;
    XpDamage = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetXpDamage(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = XpDamage;
    XpDamage = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set training queue
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the training queue
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetTrainingQueue(SCM flag)
{
    int old;

    old = EnableTrainingQueue;
    EnableTrainingQueue = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetTrainingQueue(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = EnableTrainingQueue;
    EnableTrainingQueue = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set capture buildings
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the flag
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetBuildingCapture(SCM flag)
{
    int old;

    old = EnableBuildingCapture;
    EnableBuildingCapture = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetBuildingCapture(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = EnableBuildingCapture;
    EnableBuildingCapture = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Set reveal attacker
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the flag
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetRevealAttacker(SCM flag)
{
    int old;

    old = RevealAttacker;
    RevealAttacker = gh_scm2bool(flag);

    return gh_bool2scm(old);
}
#elif defined(USE_LUA)
local int CclSetRevealAttacker(lua_State* l)
{
    int old;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    old = RevealAttacker;
    RevealAttacker = LuaToBoolean(l, 1);

    lua_pushboolean(l, old);
    return 1;
}
#endif

/**
**	Get a unit pointer
**
**	@param value	Unit slot number.
**
**	@return		The unit pointer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local Unit* CclGetUnit(SCM value)
{
    return UnitSlots[gh_scm2int(value)];
}

/**
**	Parse order
**
**	@param list	All options of the order.
**	@param order	OUT: resulting order.
*/
local void CclParseOrder(SCM list,Order* order)
{
    SCM value;
    SCM sublist;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("action-none"))) {
	    order->Action = UnitActionNone;
	} else if (gh_eq_p(value, gh_symbol2scm("action-still"))) {
	    order->Action = UnitActionStill;
	} else if (gh_eq_p(value, gh_symbol2scm("action-stand-ground"))) {
	    order->Action = UnitActionStandGround;
	} else if (gh_eq_p(value, gh_symbol2scm("action-follow"))) {
	    order->Action = UnitActionFollow;
	} else if (gh_eq_p(value, gh_symbol2scm("action-move"))) {
	    order->Action = UnitActionMove;
	} else if (gh_eq_p(value, gh_symbol2scm("action-attack"))) {
	    order->Action = UnitActionAttack;
	} else if (gh_eq_p(value, gh_symbol2scm("action-attack-ground"))) {
	    order->Action = UnitActionAttackGround;
	} else if (gh_eq_p(value, gh_symbol2scm("action-die"))) {
	    order->Action = UnitActionDie;
	} else if (gh_eq_p(value, gh_symbol2scm("action-spell-cast"))) {
	    order->Action = UnitActionSpellCast;
	} else if (gh_eq_p(value, gh_symbol2scm("action-train"))) {
	    order->Action = UnitActionTrain;
	} else if (gh_eq_p(value, gh_symbol2scm("action-upgrade-to"))) {
	    order->Action = UnitActionUpgradeTo;
	} else if (gh_eq_p(value, gh_symbol2scm("action-research"))) {
	    order->Action = UnitActionResearch;
	} else if (gh_eq_p(value, gh_symbol2scm("action-builded"))) {
	    order->Action = UnitActionBuilded;
	} else if (gh_eq_p(value, gh_symbol2scm("action-board"))) {
	    order->Action = UnitActionBoard;
	} else if (gh_eq_p(value, gh_symbol2scm("action-unload"))) {
	    order->Action = UnitActionUnload;
	} else if (gh_eq_p(value, gh_symbol2scm("action-patrol"))) {
	    order->Action = UnitActionPatrol;
	} else if (gh_eq_p(value, gh_symbol2scm("action-build"))) {
	    order->Action = UnitActionBuild;
	} else if (gh_eq_p(value, gh_symbol2scm("action-repair"))) {
	    order->Action = UnitActionRepair;
	} else if (gh_eq_p(value, gh_symbol2scm("action-resource"))) {
	    order->Action = UnitActionResource;
	} else if (gh_eq_p(value, gh_symbol2scm("action-return-goods"))) {
	    order->Action = UnitActionReturnGoods;

	} else if (gh_eq_p(value, gh_symbol2scm("flags"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    order->Flags = gh_scm2int(value);

	} else if (gh_eq_p(value, gh_symbol2scm("range"))) {
	    order->Range = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("min-range"))) {
	    order->MinRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("width"))) {
	    order->Width = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("height"))) {
	    order->Height = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("goal"))) {
	    char* str;
	    int slot;

	    value = gh_car(list);
	    list = gh_cdr(list);
	    str = gh_scm2newstr(value,NULL);

	    slot = strtol(str + 1, NULL, 16);
	    order->Goal = UnitSlots[slot];
	    if (!UnitSlots[slot]) {
		DebugLevel0Fn("FIXME: Forward reference not supported\n");
	    }
	    ++UnitSlots[slot]->Refs;
	    free(str);

	} else if (gh_eq_p(value, gh_symbol2scm("tile"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    order->X = gh_scm2int(gh_car(sublist));
	    order->Y = gh_scm2int(gh_cadr(sublist));

	} else if (gh_eq_p(value, gh_symbol2scm("type"))) {
	    char* str;

	    value = gh_car(list);
	    list = gh_cdr(list);
	    str = gh_scm2newstr(value,NULL);
	    order->Type = UnitTypeByIdent(str);
	    free(str);

	} else if (gh_eq_p(value, gh_symbol2scm("patrol"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    order->Arg1 = (void*)((gh_scm2int(gh_car(sublist)) << 16) |
		gh_scm2int(gh_cadr(sublist)));

	} else if (gh_eq_p(value, gh_symbol2scm("spell"))) {
	    char* str;

	    value = gh_car(list);
	    list = gh_cdr(list);
	    str = gh_scm2newstr(value, NULL);
	    order->Arg1 = SpellTypeByIdent(str);
	    free(str);

	} else if (gh_eq_p(value, gh_symbol2scm("upgrade"))) {
	    char* str;

	    value = gh_car(list);
	    list = gh_cdr(list);
	    str = gh_scm2newstr(value, NULL);
	    order->Arg1 = UpgradeByIdent(str);
	    free(str);

	} else if (gh_eq_p(value, gh_symbol2scm("mine"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    order->Arg1 = (void*)((gh_scm2int(gh_car(sublist)) << 16) |
		gh_scm2int(gh_cadr(sublist)));

	} else if (gh_eq_p(value, gh_symbol2scm("arg1"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    order->Arg1 = (void*)gh_scm2int(value);

	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag", value);
	}
    }
}

/**
**	Parse orders.
**
**	@param unit	Unit pointer which should get the orders.
**	@param vector	All options of the order.
*/
local void CclParseOrders(Unit* unit, SCM vector)
{
    int i;
    int n;

    n=gh_vector_length(vector);
    DebugCheck(n != MAX_ORDERS);
    for (i = 0; i < n; ++i) {
	CclParseOrder(gh_vector_ref(vector, gh_int2scm(i)), &unit->Orders[i]);
    }
}

/**
**	Parse builded
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the builded data.
*/
local void CclParseBuilded(Unit* unit, SCM list)
{
    SCM value;
    char* str;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("worker"))) {
	    int slot;

	    value = gh_car(list);
	    str = gh_scm2newstr(value, NULL);
	    slot = strtol(str + 1, NULL, 16);
	    DebugCheck(!UnitSlots[slot]);
	    unit->Data.Builded.Worker = UnitSlots[slot];
	    ++UnitSlots[slot]->Refs;
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("progress"))) {
	    unit->Data.Builded.Progress = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("cancel"))) {
	    unit->Data.Builded.Cancel = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("frame"))) {
	    int frame;
	    ConstructionFrame* cframe;

	    frame = gh_scm2int(gh_car(list));
	    cframe = unit->Type->Construction->Frames;
	    while (frame--) {
		cframe = cframe->Next;
	    }
	    unit->Data.Builded.Frame = cframe;
	    value = gh_car(list);
	    list = gh_cdr(list);
	}
    }
}

/**
**	Parse res worker data
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the resource worker data.
*/
local void CclParseResWorker(Unit* unit, SCM list)
{
    SCM value;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("time-to-harvest"))) {
	    unit->Data.ResWorker.TimeToHarvest = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("done-harvesting"))) {
	    unit->Data.ResWorker.DoneHarvesting = 1;
	    list = gh_cdr(list);
	}
    }
}

/**
**	Parse research
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the research data.
*/
local void CclParseResearch(Unit* unit, SCM list)
{
    SCM value;
    char* str;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("ident"))) {
	    value = gh_car(list);
	    str = gh_scm2newstr(value, NULL);
	    unit->Data.Research.Upgrade = UpgradeByIdent(str);
	    free(str);
	    list = gh_cdr(list);
	}
    }
}

/**
**	Parse upgrade to
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the upgrade to data.
*/
local void CclParseUpgradeTo(Unit* unit, SCM list)
{
    SCM value;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("ticks"))) {
	    unit->Data.UpgradeTo.Ticks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	}
    }
}

/**
**	Parse stored data for train order
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the trained order
*/
local void CclParseTrain(Unit *unit, SCM list)
{
    SCM value;
    SCM sublist;
    int i;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("ticks"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    unit->Data.Train.Ticks = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("count"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    unit->Data.Train.Count = gh_scm2int(value);
	} else if (gh_eq_p(value, gh_symbol2scm("queue"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    for (i = 0; i < MAX_UNIT_TRAIN; ++i) {
		value = gh_vector_ref(sublist, gh_int2scm(i));
		if (gh_eq_p(value, gh_symbol2scm("unit-none"))) {
		    unit->Data.Train.What[i] = NULL;
		} else {
		    char* ident;
		    ident = gh_scm2newstr(value, NULL);
		    unit->Data.Train.What[i] = UnitTypeByIdent(ident);
		    free(ident);
		}
	    }
	}
    }
}

/**
**	Parse stored data for move order
**
**	@param unit	Unit pointer which should be filled with the data.
**	@param list	All options of the move order
*/
local void CclParseMove(Unit *unit, SCM list)
{
    SCM value;
    SCM sublist;
    int i;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("fast"))) {
	    unit->Data.Move.Fast = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("path"))) {
	    int len;
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    len = gh_length(sublist);
	    for (i = 0; i < len; ++i) {
		value = gh_vector_ref(sublist, gh_int2scm(i));
		unit->Data.Move.Path[i] = gh_scm2int(value);
	    }
	    unit->Data.Move.Length = len;
	}
    }
}

/**
**	Parse unit
**
**	@param list	List describing unit
*/
local SCM CclUnit(SCM list)
{
    SCM value;
    SCM sublist;
    Unit* unit;
    UnitType* type;
    UnitType* seentype;
    Player* player;
    int slot;
    int i;
    int insidecount;
    char* str;

    insidecount = -1;
    slot = gh_scm2int(gh_car(list));
    list = gh_cdr(list);
    DebugLevel3Fn("parsing unit #%d\n" _C_ slot);

    unit = NULL;
    type = NULL;
    seentype = NULL;
    player = NULL;
    i = 0;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {

	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("type"))) {
	    type=UnitTypeByIdent(str = gh_scm2newstr(gh_car(list),NULL));
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    unit->Name = gh_scm2newstr(gh_car(list),NULL);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("player"))) {
	    player = &Players[gh_scm2int(gh_car(list))];
	    list = gh_cdr(list);

	    // During a unit's death animation (when action is "die" but the
	    // unit still has its original type, i.e. it's still not a corpse)
	    // the unit is already removed from map and from player's
	    // unit list (=the unit went through LetUnitDie() which
	    // calls RemoveUnit() and UnitLost()).  Such a unit should not
	    // be put on player's unit list!  However, this state is not
	    // easily detected from this place.  It seems that it is
	    // characterized by unit->HP==0 and by
	    // unit->Orders[0].Action==UnitActionDie so we have to wait
	    // until we parsed at least Unit::Orders[].
	    DebugCheck(!type);
	    unit = UnitSlots[slot];
	    InitUnit(unit, type);
	    unit->SeenType = seentype;
	    unit->Active = 0;
	    unit->Removed = 0;
	    unit->Reset = 0;		// JOHNS ????
	    DebugCheck(unit->Slot != slot);
	} else if (gh_eq_p(value, gh_symbol2scm("next"))) {
	    unit->Next = UnitSlots[gh_scm2int(gh_car(list))];
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("current-sight-range"))) {
	    unit->CurrentSightRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("host-info"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    MapMarkSight(player, gh_scm2int(gh_car(value)), gh_scm2int(gh_cadr(value)),
		gh_scm2int(gh_cadr(gh_cdr(value))),
		gh_scm2int(gh_cadr(gh_cdr(gh_cdr(value)))),
		unit->CurrentSightRange);
	} else if (gh_eq_p(value, gh_symbol2scm("tile"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    unit->X = gh_scm2int(gh_car(value));
	    unit->Y = gh_scm2int(gh_cadr(value));
	} else if (gh_eq_p(value, gh_symbol2scm("stats"))) {
	    unit->Stats = &type->Stats[gh_scm2int(gh_car(list))];
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("pixel"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    unit->IX = gh_scm2int(gh_car(value));
	    unit->IY = gh_scm2int(gh_cadr(value));
	} else if (gh_eq_p(value, gh_symbol2scm("frame"))) {
	    unit->Frame = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("flipped-frame"))) {
	    unit->Frame = -gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	//
	//	Here is the seen stuff.
	//
	} else if (gh_eq_p(value, gh_symbol2scm("seen"))) {
	    unit->SeenFrame = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("flipped-seen"))) {
	    unit->SeenFrame = -gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("not-seen"))) {
	    unit->SeenFrame = UnitNotSeen;
	} else if (gh_eq_p(value, gh_symbol2scm("seen-type"))) {
	    unit->SeenType = seentype = UnitTypeByIdent(str = gh_scm2newstr(gh_car(list),NULL));
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("seen-pixel"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    unit->SeenIX = gh_scm2int(gh_car(value));
	    unit->SeenIY = gh_scm2int(gh_cadr(value));
	} else if (gh_eq_p(value, gh_symbol2scm("seen-destroyed"))) {
	    unit->SeenDestroyed = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("seen-constructed"))) {
	    unit->SeenConstructed = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("seen-state"))) {
	    unit->SeenState = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("seen-construction-frame"))) {
	    int frame;
	    frame = gh_scm2int(gh_car(list));
	    unit->SeenCFrame = unit->Type->Construction->Frames;
	    while (frame--) {
		unit->SeenCFrame = unit->SeenCFrame->Next;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("vis-count"))) {
	    sublist = gh_car(list);
	    for (i = 0; i < PlayerMax; ++i) {
	    	value = gh_vector_ref(sublist, gh_int2scm(i));
		if (!gh_null_p(value)) {
		    unit->VisCount[i] = gh_scm2int(value);
		}
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("direction"))) {
	    unit->Direction = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("attacked"))) {
	    // FIXME : unsigned long should be better handled
	    unit->Attacked = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("burning"))) {
	    unit->Burning = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("destroyed"))) {
	    unit->Destroyed = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("removed"))) {
	    unit->Removed = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("selected"))) {
	    unit->Selected = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("rescued-from"))) {
	    unit->RescuedFrom = &Players[gh_scm2int(gh_car(list))];
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("constructed"))) {
	    unit->Constructed = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("active"))) {
	    unit->Active = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("resource-active"))) {
	    unit->Data.Resource.Active = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("mana"))) {
	    unit->Mana = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("hp"))) {
	    unit->HP = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("xp"))) {
	    unit->XP = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("kills"))) {
	    unit->Kills = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
	    // FIXME : unsigned long should be better handled
	    unit->TTL = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("bloodlust"))) {
	    unit->Bloodlust = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("haste"))) {
	    unit->Haste = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("slow"))) {
	    unit->Slow = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("invisible"))) {
	    unit->Invisible = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("flame-shield"))) {
	    unit->FlameShield = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("unholy-armor"))) {
	    unit->UnholyArmor = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("group-id"))) {
	    unit->GroupId = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("last-group"))) {
	    unit->LastGroup = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("value"))) {
	    unit->Value = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("current-resource"))) {
	    unit->CurrentResource = CclGetResourceByName(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("sub-action"))) {
	    unit->SubAction = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("wait"))) {
	    unit->Wait = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("state"))) {
	    unit->State = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("reset"))) {
	    unit->Reset = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("blink"))) {
	    unit->Blink = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("moving"))) {
	    unit->Moving = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("rs"))) {
	    unit->Rs = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("units-contained-count"))) {
	    insidecount = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("units-contained"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    for (i = 0; i < insidecount; ++i) {
	    	value = gh_vector_ref(sublist, gh_int2scm(i));
		if (!gh_null_p(value)) {
		    str = gh_scm2newstr(value, NULL);
		    slot = strtol(str + 1, NULL, 16);
		    AddUnitInContainer(UnitSlots[slot], unit);
		    DebugCheck(!UnitSlots[slot]);
		    ++UnitSlots[slot]->Refs;
		    free(str);
		}
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("order-count"))) {
	    unit->OrderCount = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("order-flush"))) {
	    unit->OrderFlush = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("orders"))) {
	    int hp;
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseOrders(unit,sublist);
	    // now we know unit's action so we can assign it to a player
	    hp = unit->HP;
	    AssignUnitToPlayer (unit, player);
	    unit->HP = hp;
	    if (unit->Orders[0].Action == UnitActionBuilded) {
		// HACK: the building is not ready yet
		unit->Player->UnitTypesCount[type->Type]--;
	    }
	    // FIXME: (mr-russ) Does not load CorpseList Properly
	    if (unit->Orders[0].Action == UnitActionDie) {
		CorpseCacheInsert(unit);
	    }
#if 0
	    if (unit->Orders[0].Action == UnitActionDie &&
		    unit->Type->CorpseScript) {
		MapMarkUnitSight(unit);
	    }
#endif
	} else if (gh_eq_p(value, gh_symbol2scm("saved-order"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    CclParseOrder(value, &unit->SavedOrder);
	} else if (gh_eq_p(value, gh_symbol2scm("new-order"))) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    CclParseOrder(value, &unit->NewOrder);
	} else if (gh_eq_p(value, gh_symbol2scm("data-builded"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseBuilded(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("data-res-worker"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseResWorker(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("data-research"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseResearch(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("data-upgrade-to"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseUpgradeTo(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("data-train"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseTrain(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("data-move"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    CclParseMove(unit, sublist);
	} else if (gh_eq_p(value, gh_symbol2scm("goal"))) {
	    unit->Goal=UnitSlots[gh_scm2int(gh_car(list))];
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("auto-cast"))) {
	    str = gh_scm2newstr(gh_car(list),NULL);
	    unit->AutoCastSpell = SpellTypeByIdent(str);
	    list = gh_cdr(list);
	    free(str);
	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag", value);
	}
    }

    if (!unit->Player) {
	AssignUnitToPlayer (unit, player);
	UpdateForNewUnit(unit, 0);
	unit->HP = unit->Type->_HitPoints;
    }

    //  Revealers are units that can see while removed
    if (unit->Removed && unit->Type->Revealer) {
    	MapMarkUnitSight(unit);
    }

    //	Place on map
    if (!unit->Removed && !unit->Destroyed && !unit->Type->Vanishes) {
	unit->Removed = 1;
	PlaceUnit(unit, unit->X, unit->Y);
    }

    // FIXME: johns: works only for debug code.
    if (unit->Moving) {
	NewResetPath(unit);
    }
    // Fix Colors for rescued units.
    if (unit->RescuedFrom) {
	unit->Colors = &unit->RescuedFrom->UnitColors;
    }
    DebugLevel3Fn("unit #%d parsed\n" _C_ slot);

    return SCM_UNSPECIFIED;
}

/**
**	Make a unit.
**
**	@param type	Unit-type of the unit,
**	@param player	Owning player number of the unit.
**
**	@return		Returns the slot number of the made unit.
*/
local SCM CclMakeUnit(SCM type, SCM player)
{
    UnitType* unittype;
    Unit* unit;

    unittype = CclGetUnitType(type);
    unit = MakeUnit(unittype, &Players[gh_scm2int(player)]);

    return gh_int2scm(unit->Slot);
}

/**
**	Place a unit on map.
**
**	@param unit	Unit (slot number) to be placed.
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		Returns the slot number of the made placed.
*/
local SCM CclPlaceUnit(SCM unit, SCM x, SCM y)
{
    PlaceUnit(CclGetUnit(unit), gh_scm2int(x), gh_scm2int(y));
    return unit;
}

/**
**	Create a unit and place it on the map
**
**	@param type	Unit-type of the unit,
**	@param player	Owning player number of the unit.
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		Returns the slot number of the made unit.
*/
local SCM CclCreateUnit(SCM type, SCM player, SCM x, SCM y)
{
    UnitType* unittype;
    Unit* unit;
    int heading;
    int playerno;
    int mask;
    int ix;
    int iy;

    unittype = CclGetUnitType(type);
    ix = gh_scm2int(x);
    iy = gh_scm2int(y);

    heading = SyncRand() % 256;
    playerno = TriggerGetPlayer(player);
    if (playerno == -1) {
	printf("CreateUnit: You cannot use 'any in create-unit, specify a player\n");
	errl("bad player", player);
	return SCM_UNSPECIFIED;
    }
    unit = MakeUnit(unittype, &Players[playerno]);
    mask = UnitMovementMask(unit);
    if (CheckedCanMoveToMask(ix, iy, mask)) {
	unit->Wait = 1;
	PlaceUnit(unit, ix, iy);
    } else {
	unit->X = ix;
	unit->Y = iy;
	DropOutOnSide(unit, heading, unittype->TileWidth, unittype->TileHeight);
    }

    return gh_int2scm(unit->Slot);
}

/**
**	Order a unit
**
**	(order-unit player unit-type sloc dloc order)
*/
local SCM CclOrderUnit(SCM list)
{
    int plynr;
    int x1;
    int y1;
    int x2;
    int y2;
    int dx1;
    int dy1;
    int dx2;
    int dy2;
    const UnitType* unittype;
    Unit* table[UnitMax];
    Unit* unit;
    int an;
    int j;
    char* order;

    plynr = TriggerGetPlayer(gh_car(list));
    list = gh_cdr(list);
    unittype = TriggerGetUnitType(gh_car(list));
    list = gh_cdr(list);
    x1 = gh_scm2int(gh_car(gh_car(list)));
    y1 = gh_scm2int(gh_car(gh_cdr(gh_car(list))));
    if (!gh_null_p(gh_cdr(gh_cdr(gh_car(list))))) {
	x2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(gh_car(list)))));
	y2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(gh_cdr(gh_car(list))))));
    } else {
	x2 = x1;
	y2 = y1;
    }
    list = gh_cdr(list);
    dx1 = gh_scm2int(gh_car(gh_car(list)));
    dy1 = gh_scm2int(gh_car(gh_cdr(gh_car(list))));
    if (!gh_null_p(gh_cdr(gh_cdr(gh_car(list))))) {
	dx2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(gh_car(list)))));
	dy2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(gh_cdr(gh_car(list))))));
    } else {
	dx2 = dx1;
	dy2 = dy1;
    }
    list = gh_cdr(list);
    order = gh_scm2newstr(gh_car(list),NULL);

    an = SelectUnits(x1, y1, x2 + 1, y2 + 1, table);
    for (j = 0; j < an; ++j) {
	unit = table[j];
	if (unittype == ANY_UNIT ||
		(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
		(unittype == ALL_BUILDINGS && unit->Type->Building) ||
		unittype == unit->Type) {
	    if (plynr == -1 || plynr == unit->Player->Player) {
		if (!strcmp(order,"move")) {
		    CommandMove(unit, (dx1 + dx2) / 2, (dy1 + dy2) / 2, 1);
		} else if (!strcmp(order, "attack")) {
		    Unit* attack;

		    attack=TargetOnMap(unit, dx1, dy1, dx2, dy2);
		    CommandAttack(unit, (dx1 + dx2) / 2, (dy1 + dy2) / 2, attack, 1);
		} else if (!strcmp(order,"patrol")) {
		    CommandPatrolUnit(unit, (dx1 + dx2) / 2, (dy1 + dy2) / 2, 1);
		} else {
		    errl("Unsupported order", gh_car(list));
		}
	    }
	}
    }

    free(order);
    return SCM_UNSPECIFIED;
}

/**
**	Kill a unit
**
**	@param type	Unit-type of the unit,
**	@param player	Owning player number of the unit.
**
**	@return		Returns true if a unit was killed.
*/
local SCM CclKillUnit(SCM type, SCM player)
{
    int j;
    int plynr;
    const UnitType* unittype;
    Unit* unit;
    Unit** table;

    unittype = TriggerGetUnitType(type);
    plynr = TriggerGetPlayer(player);
    if (plynr == -1) {
	table = Units;
	j = NumUnits - 1;
    } else {
	table = Players[plynr].Units;
	j = Players[plynr].TotalNumUnits - 1;
    }

    for (; j >= 0; --j) {
	unit = table[j];
	if (unittype == ANY_UNIT ||
		(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
		(unittype == ALL_BUILDINGS && unit->Type->Building) ||
		unittype == unit->Type) {
	    LetUnitDie(unit);
	    return SCM_BOOL_T;
	}
    }

    return SCM_BOOL_F;
}

/**
**	Kill a unit at a location
**
**	@param type	Unit-type of the unit,
**	@param player	Owning player number of the unit.
**	@param quantity	Number of units to kill.
**	@param loc	Co-ordinates list (x1 y1 x2 y2)
**
**	@return		Returns the number of units killed.
*/
local SCM CclKillUnitAt(SCM type, SCM player, SCM quantity, SCM loc)
{
    int plynr;
    int q;
    int x1;
    int y1;
    int x2;
    int y2;
    const UnitType* unittype;
    Unit* table[UnitMax];
    Unit* unit;
    int an;
    int j;
    int s;

    plynr=TriggerGetPlayer(player);
    q = gh_scm2int(quantity);
    unittype = TriggerGetUnitType(type);
    x1 = gh_scm2int(gh_car(loc));
    y1 = gh_scm2int(gh_car(gh_cdr(loc)));
    x2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(loc))));
    y2 = gh_scm2int(gh_car(gh_cdr(gh_cdr(gh_cdr(loc)))));

    an=SelectUnits(x1, y1, x2 + 1, y2 + 1, table);
    for (j = s = 0; j < an && s < q; ++j) {
	unit = table[j];
	if (unittype == ANY_UNIT ||
		(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
		(unittype == ALL_BUILDINGS && unit->Type->Building) ||
		unittype==unit->Type) {
	    if (plynr == -1 || plynr == unit->Player->Player) {
		LetUnitDie(unit);
		++s;
	    }
	}
    }

    return gh_int2scm(s);
}

/**
**	Get the unholy-armor of the unit structure.
**
**	@param ptr	Unit object.
**
**	@return		The unholy-armor of the unit.
*/
local SCM CclGetUnitUnholyArmor(SCM ptr)
{
    const Unit* unit;
    SCM value;

    unit = CclGetUnit(ptr);
    value = gh_int2scm(unit->UnholyArmor);
    return value;
}

/**
**	Set the unholy-armor of the unit structure.
**
**	@param ptr	Unit object.
**	@param value	The value to set.
**
**	@return		The value of the unit.
*/
local SCM CclSetUnitUnholyArmor(SCM ptr, SCM value)
{
    Unit* unit;

    unit = CclGetUnit(ptr);
    unit->UnholyArmor = gh_scm2int(value);

    return value;
}

local SCM CclSlotUsage(SCM list)
{
#define SLOT_LEN MAX_UNIT_SLOTS / 8 + 1
    unsigned char SlotUsage[SLOT_LEN];
    int i;
    int prev;
    SCM value;

    memset(SlotUsage, 0, SLOT_LEN);
    prev = -1;
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("-"))) {
	    int range_end;
	    value = gh_car(list);
	    list = gh_cdr(list);
	    range_end = gh_scm2int(value);
	    for (i = prev; i <= range_end; ++i) {
		SlotUsage[i / 8] |= 1 << (i % 8);
	    }
	    prev = -1;
	} else {
	    if (prev >= 0) {
		SlotUsage[prev / 8] |= 1 << (prev % 8);
	    }
	    prev = gh_scm2int(value);
	}
    }
    if (prev >= 0) {
	SlotUsage[prev / 8] |= 1 << (prev % 8);
    }

    /* now walk through the bitfield and create the needed unit slots */
    for (i = 0; i < SLOT_LEN * 8; ++i) {
	if (SlotUsage[i / 8] & (1 << i % 8)) {
	    Unit* new_unit = (Unit*)calloc(1, sizeof(Unit));
	    UnitSlotFree = (void*)UnitSlots[i];
	    UnitSlots[i] = new_unit;
	    new_unit->Slot = i;
	}
    }
    return SCM_UNSPECIFIED;
#undef SLOT_LEN
}
#elif defined(USE_LUA)
#endif

// FIXME: write the missing access functions

/**
**	Register CCL features for unit.
*/
global void UnitCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedure1_0("set-xp-damage!", CclSetXpDamage);
    gh_new_procedure1_0("set-training-queue!", CclSetTrainingQueue);
    gh_new_procedure1_0("set-building-capture!", CclSetBuildingCapture);
    gh_new_procedure1_0("set-reveal-attacker!", CclSetRevealAttacker);

    gh_new_procedureN("unit", CclUnit);

    gh_new_procedure2_0("make-unit", CclMakeUnit);
    gh_new_procedure3_0("place-unit", CclPlaceUnit);
    gh_new_procedure4_0("create-unit", CclCreateUnit);
    gh_new_procedureN("order-unit", CclOrderUnit);
    gh_new_procedure2_0("kill-unit", CclKillUnit);
    gh_new_procedure4_0("kill-unit-at", CclKillUnitAt);

    // unit member access functions
    gh_new_procedure1_0("get-unit-unholy-armor", CclGetUnitUnholyArmor);
    gh_new_procedure2_0("set-unit-unholy-armor!", CclSetUnitUnholyArmor);

    gh_new_procedure1_0 ("slot-usage", CclSlotUsage);
#elif defined(USE_LUA)
    lua_register(Lua, "SetXPDamage", CclSetXpDamage);
    lua_register(Lua, "SetTrainingQueue", CclSetTrainingQueue);
    lua_register(Lua, "SetBuildingCapture", CclSetBuildingCapture);
    lua_register(Lua, "SetRevealAttacker", CclSetRevealAttacker);

//    lua_register(Lua, "Unit", CclUnit);

//    lua_register(Lua, "MakeUnit", CclMakeUnit);
//    lua_register(Lua, "PlaceUnit", CclPlaceUnit);
//    lua_register(Lua, "CreateUnit", CclCreateUnit);
//    lua_register(Lua, "OrderUnit", CclOrderUnit);
//    lua_register(Lua, "KillUnit", CclKillUnit);
//    lua_register(Lua, "KillUnitAt", CclKillUnitAt);

    // unit member access functions
//    lua_register(Lua, "GetUnitUnholyArmor", CclGetUnitUnholyArmor);
//    lua_register(Lua, "SetUnitUnholyArmor", CclSetUnitUnholyArmor);

//    lua_register(Lua, "SlotUsage", CclSlotUsage);
#endif
}

//@}
