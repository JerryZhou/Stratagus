//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name commands.c	-	Global command handler - network support. */
//
//	(c) Copyright 2000,2001 by Lutz Sammer and Andreas Arens.
//
//	$Id$

//@{

//----------------------------------------------------------------------------
//	Includes
//----------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>

#include "freecraft.h"
#include "unit.h"
#include "map.h"
#include "actions.h"
#include "player.h"
#include "network.h"

//----------------------------------------------------------------------------
//	Declaration
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	Variables
//----------------------------------------------------------------------------

global int CommandLogEnabled;		/// True if command log is on

//----------------------------------------------------------------------------
//	Log commands
//----------------------------------------------------------------------------

/**@name log */
//@{

/**
**	Log commands into file.
**
**	This could later be used to recover, crashed games.
**
**	@param name	Command name (move,attack,...).
**	@param unit	Unit that receive the command.
**	@param flag	Append command or flush old commands.
**	@param x	optional X map position.
**	@param y	optional y map position.
**	@param dest	optional destination unit.
**	@param type	optional command argument (unit-type,...).
**	@param num	optional number argument
*/
local void CommandLog(const char* name,const Unit* unit,int flag,
	unsigned x,unsigned y,const Unit* dest,const char* value,int num)
{
    static FILE* logf;
    extern unsigned SyncRandSeed;

    if( !CommandLogEnabled ) {
	return;
    }

    //
    //	Create and write header of log file.
    //
    if( !logf ) {
	time_t now;
	char buf[256];

	sprintf(buf,"log_of_freecraft_%d.log",ThisPlayer->Player);
	logf=fopen(buf,"wb");
	if( !logf ) {
	    return;
	}
	fprintf(logf,";;; Log file generated by FreeCraft Version "
		VERSION "\n");
	time(&now);
	fprintf(logf,";;;\tDate: %s",ctime(&now));
	fprintf(logf,";;;\tMap: %s\n\n",TheMap.Description);
    }

    //
    //	Frame, unit, (type-ident only for better readable.
    //
    fprintf(logf,"(log %d 'U%Zd '%s '%s '%s",
	    FrameCounter,UnitNumber(unit),unit->Type->Ident,name,
	    flag ? "flush" : "append");

    //
    //	Coordinates given.
    //
    if( x!=-1 || y!=-1 ) {
	fprintf(logf," (%d %d)",x,y);
    }
    //
    //	Destination given.
    //
    if( dest ) {
	fprintf(logf," 'U%Zd",UnitNumber(dest));
    }
    //
    //	Value given.
    //
    if( value ) {
	fprintf(logf," '%s",value);
    }
    //
    //	Number given.
    //
    if( num!=-1 ) {
	fprintf(logf," %d",num);
    }
    fprintf(logf,") ;%d %d",unit->Refs,SyncRandSeed);

    fprintf(logf,"\n");
    fflush(logf);
}

//@}

//----------------------------------------------------------------------------
//	Send game commands, maybe over the network.
//----------------------------------------------------------------------------

/**@name send */
//@{

/**
**	Send command: Unit stop.
**
**	@param unit	pointer to unit.
*/
global void SendCommandStopUnit(Unit* unit)
{
    if( NetworkFildes==-1 ) {
	CommandLog("stop",unit,FlushCommands,-1,-1,NoUnitP,NULL,-1);
	CommandStopUnit(unit);
    } else {
	NetworkSendCommand(MessageCommandStop,unit,0,0,NoUnitP,0,FlushCommands);
    }
}

/**
**	Send command: Unit stand ground.
**
**	@param unit	pointer to unit.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandStandGround(Unit* unit,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("stand-ground",unit,flush,-1,-1,NoUnitP,NULL,-1);
	CommandStandGround(unit,flush);
    } else {
	NetworkSendCommand(MessageCommandStand,unit,0,0,NoUnitP,0,flush);
    }
}

/**
**	Send command: Follow unit to position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to move to.
**	@param y	Y map tile position to move to.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandFollow(Unit* unit,Unit* dest,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("follow",unit,flush,-1,-1,dest,NULL,-1);
	CommandFollow(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandFollow,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Move unit to position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to move to.
**	@param y	Y map tile position to move to.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandMove(Unit* unit,int x,int y,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("move",unit,flush,x,y,NoUnitP,NULL,-1);
	CommandMove(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandMove,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit repair.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to repair.
**	@param y	Y map tile position to repair.
**	@param dest	Unit to be repaired.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandRepair(Unit* unit,int x,int y,Unit* dest,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("repair",unit,flush,x,y,dest,NULL,-1);
	CommandRepair(unit,x,y,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandRepair,unit,x,y,dest,0,flush);
    }
}

/**
**	Send command: Unit attack unit or at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to attack.
**	@param y	Y map tile position to attack.
**	@param attack	or !=NoUnitP unit to be attacked.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandAttack(Unit* unit,int x,int y,Unit* attack,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("attack",unit,flush,x,y,attack,NULL,-1);
	CommandAttack(unit,x,y,attack,flush);
    } else {
	NetworkSendCommand(MessageCommandAttack,unit,x,y,attack,0,flush);
    }
}

/**
**	Send command: Unit attack ground.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to fire on.
**	@param y	Y map tile position to fire on.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandAttackGround(Unit* unit,int x,int y,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("attack-ground",unit,flush,x,y,NoUnitP,NULL,-1);
	CommandAttackGround(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandGround,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit patrol between current and position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position to patrol between.
**	@param y	Y map tile position to patrol between.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandPatrol(Unit* unit,int x,int y,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("patrol",unit,flush,x,y,NoUnitP,NULL,-1);
	CommandPatrolUnit(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandPatrol,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit board unit.
**
**	@param unit	pointer to unit.
**	@param dest	Destination to be boarded.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandBoard(Unit* unit,int x,int y,Unit* dest,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("board",unit,flush,x,y,dest,NULL,-1);
	CommandBoard(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandBoard,unit,x,y,dest,0,flush);
    }
}

/**
**	Send command: Unit unload unit.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position of unload.
**	@param y	Y map tile position of unload.
**	@param what	Passagier to be unloaded.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandUnload(Unit* unit,int x,int y,Unit* what,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("unload",unit,flush,x,y,what,NULL,-1);
	CommandUnload(unit,x,y,what,flush);
    } else {
	NetworkSendCommand(MessageCommandUnload,unit,x,y,what,0,flush);
    }
}

/**
**	Send command: Unit builds building at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position of construction.
**	@param y	Y map tile position of construction.
**	@param what	pointer to unit-type of the building.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandBuildBuilding(Unit* unit,int x,int y
	,UnitType* what,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("build",unit,flush,x,y,NULL,what->Ident,-1);
	CommandBuildBuilding(unit,x,y,what,flush);
    } else {
	NetworkSendCommand(MessageCommandBuild,unit,x,y,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel this building construction.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelBuilding(Unit* unit,Unit* worker)
{
    // FIXME: currently unit and worker are same?
    if( NetworkFildes==-1 ) {
	CommandLog("cancel-build",unit,FlushCommands,-1,-1,worker,NULL,-1);
	CommandCancelBuilding(unit,worker);
    } else {
	NetworkSendCommand(MessageCommandCancelBuild,unit,0,0,worker,0
		,FlushCommands);
    }
}

/**
**	Send command: Unit harvest wood.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position where to harvest.
**	@param y	Y map tile position where to harvest.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandHarvest(Unit* unit,int x,int y,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("harvest",unit,flush,x,y,NoUnitP,NULL,-1);
	CommandHarvest(unit,x,y,flush);
    } else {
	NetworkSendCommand(MessageCommandHarvest,unit,x,y,NoUnitP,0,flush);
    }
}

/**
**	Send command: Unit mine gold.
**
**	@param unit	pointer to unit.
**	@param dest	pointer to destination (gold-mine).
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandMineGold(Unit* unit,Unit* dest,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("mine",unit,flush,-1,-1,dest,NULL,-1);
	CommandMineGold(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandMine,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Unit haul oil.
**
**	@param unit	pointer to unit.
**	@param dest	pointer to destination (oil-platform).
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandHaulOil(Unit* unit,Unit* dest,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("haul",unit,flush,-1,-1,dest,NULL,-1);
	CommandHaulOil(unit,dest,flush);
    } else {
	NetworkSendCommand(MessageCommandHaul,unit,0,0,dest,0,flush);
    }
}

/**
**	Send command: Unit return goods.
**
**	@param unit	pointer to unit.
**	@param goal	pointer to destination of the goods. (NULL=search best)
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandReturnGoods(Unit* unit,Unit* goal,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("return",unit,flush,-1,-1,goal,NULL,-1);
	CommandReturnGoods(unit,goal,flush);
    } else {
	NetworkSendCommand(MessageCommandReturn,unit,0,0,goal,0,flush);
    }
}

/**
**	Send command: Building/unit train new unit.
**
**	@param unit	pointer to unit.
**	@param what	pointer to unit-type of the unit to be trained.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandTrainUnit(Unit* unit,UnitType* what,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("train",unit,flush,-1,-1,NULL,what->Ident,-1);
	CommandTrainUnit(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandTrain,unit,0,0,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel training.
**
**	@param unit	Pointer to unit.
**	@param slot	Slot of training queue to cancel.
*/
global void SendCommandCancelTraining(Unit* unit,int slot)
{
    if( NetworkFildes==-1 ) {
	CommandLog("cancel-train",unit,FlushCommands,-1,-1,NoUnitP,NULL,slot);
	CommandCancelTraining(unit,slot);
    } else {
	NetworkSendCommand(MessageCommandCancelTrain,unit,slot,0,NoUnitP,NULL
		,FlushCommands);
    }
}

/**
**	Send command: Building starts upgrading to.
**
**	@param unit	pointer to unit.
**	@param what	pointer to unit-type of the unit upgrade.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandUpgradeTo(Unit* unit,UnitType* what,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("upgrade-to",unit,flush,-1,-1,NULL,what->Ident,-1);
	CommandUpgradeTo(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandUpgrade,unit,0,0,NoUnitP,what,flush);
    }
}

/**
**	Send command: Cancel building upgrading to.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelUpgradeTo(Unit* unit)
{
    if( NetworkFildes==-1 ) {
	CommandLog("cancel-upgrade-to",unit,FlushCommands
		,-1,-1,NoUnitP,NULL,-1);
	CommandCancelUpgradeTo(unit);
    } else {
	NetworkSendCommand(MessageCommandCancelUpgrade,unit
		,0,0,NoUnitP,NULL,FlushCommands);
    }
}

/**
**	Send command: Building/unit research.
**
**	@param unit	pointer to unit.
**	@param what	research-type of the research.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandResearch(Unit* unit,Upgrade* what,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("research",unit,flush,-1,-1,NULL,what->Ident,-1);
	CommandResearch(unit,what,flush);
    } else {
	NetworkSendCommand(MessageCommandResearch,unit
		,what-Upgrades,0,NoUnitP,NULL,flush);
    }
}

/**
**	Send command: Cancel Building/unit research.
**
**	@param unit	pointer to unit.
*/
global void SendCommandCancelResearch(Unit* unit)
{
    if( NetworkFildes==-1 ) {
	CommandLog("cancel-research",unit,FlushCommands,-1,-1,NoUnitP,NULL,-1);
	CommandCancelResearch(unit);
    } else {
	NetworkSendCommand(MessageCommandCancelResearch,unit
		,0,0,NoUnitP,NULL,FlushCommands);
    }
}

/**
**	Send command: Unit demolish at position.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position where to demolish.
**	@param y	Y map tile position where to demolish.
**	@param attack	or !=NoUnitP unit to be demolished.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandDemolish(Unit* unit,int x,int y,Unit* attack,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("demolish",unit,flush,x,y,attack,NULL,-1);
	CommandDemolish(unit,x,y,attack,flush);
    } else {
	NetworkSendCommand(MessageCommandDemolish,unit,x,y,attack,NULL,flush);
    }
}

/**
**	Send command: Unit spell cast on position/unit.
**
**	@param unit	pointer to unit.
**	@param x	X map tile position where to cast spell.
**	@param y	Y map tile position where to cast spell.
**	@param attack	Cast spell on unit (if exist).
**	@param spellid  Spell type id.
**	@param flush	Flag flush all pending commands.
*/
global void SendCommandSpellCast(Unit* unit,int x,int y,Unit* dest,int spellid
	,int flush)
{
    if( NetworkFildes==-1 ) {
	CommandLog("spell-cast",unit,flush,x,y,dest,NULL,spellid);
	CommandSpellCast(unit,x,y,dest,SpellTypeById(spellid),flush);
    } else {
	NetworkSendCommand(MessageCommandSpellCast+spellid
		,unit,x,y,dest,NULL,flush);
    }
}

//@}

//----------------------------------------------------------------------------
//	Parse the message, from the network.
//----------------------------------------------------------------------------

/**@name parse */
//@{

/**
**	Parse a command (from network).
**
**	@param msgnr	Network message type
**	@param unum	Unit number (slot) that receive the command.
**	@param x	optional X map position.
**	@param y	optional y map position.
**	@param dstnr	optional destination unit.
*/
global void ParseCommand(unsigned short msgnr,UnitRef unum,
	unsigned short x,unsigned short y,UnitRef dstnr)
{
    Unit *unit, *dest;
    int id, status;

    DebugLevel3Fn(" %d frame %d\n", msgnr, FrameCounter);

    unit=UnitSlots[unum];
    DebugCheck( !unit );
    //
    //	Check if unit is already killed?
    //
    if( unit->Destroyed ) {
	DebugLevel0Fn(" destroyed unit skipping %Zd\n",UnitNumber(unit));
	return;
    }

    DebugCheck( !unit->Type );

    status=(msgnr&0x80)>>7;

    // Note: destroyed destination unit is handled by the action routines.

    switch( msgnr&0x7F ) {
	case MessageSync:
	    return;
	case MessageQuit:
	    return;
	case MessageChat:
	    return;

	case MessageCommandStop:
	    CommandLog("stop",unit,FlushCommands,-1,-1,NoUnitP,NULL,-1);
	    CommandStopUnit(unit);
	    break;
	case MessageCommandStand:
	    CommandLog("stand-ground",unit,status,-1,-1,NoUnitP,NULL,-1);
	    CommandStandGround(unit,status);
	    break;
	case MessageCommandFollow:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("follow",unit,status,-1,-1,dest,NULL,-1);
	    CommandFollow(unit,dest,status);
	    break;
	case MessageCommandMove:
	    CommandLog("move",unit,status,x,y,NoUnitP,NULL,-1);
	    CommandMove(unit,x,y,status);
	    break;
	case MessageCommandRepair:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("repair",unit,status,x,y,NoUnitP,NULL,-1);
	    CommandRepair(unit,x,y,dest,status);
	    break;
	case MessageCommandAttack:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("attack",unit,status,x,y,dest,NULL,-1);
	    CommandAttack(unit,x,y,dest,status);
	    break;
	case MessageCommandGround:
	    CommandLog("attack-ground",unit,status,x,y,NoUnitP,NULL,-1);
	    CommandAttackGround(unit,x,y,status);
	    break;
	case MessageCommandPatrol:
	    CommandLog("patrol",unit,status,x,y,NoUnitP,NULL,-1);
	    CommandPatrolUnit(unit,x,y,status);
	    break;
	case MessageCommandBoard:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("board",unit,status,x,y,dest,NULL,-1);
	    CommandBoard(unit,dest,status);
	    break;
	case MessageCommandUnload:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("unload",unit,status,x,y,dest,NULL,-1);
	    CommandUnload(unit,x,y,dest,status);
	    break;
	case MessageCommandBuild:
	    CommandLog("build",unit,status,x,y,NULL,UnitTypes[dstnr].Ident,-1);
	    CommandBuildBuilding(unit,x,y,UnitTypes+dstnr,status);
	    break;
	case MessageCommandCancelBuild:
	    // dest is the worker building the unit...
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("cancel-build",unit,FlushCommands,-1,-1,dest,NULL,-1);
	    CommandCancelBuilding(unit,dest);
	    break;
	case MessageCommandHarvest:
	    CommandLog("harvest",unit,status,x,y,NoUnitP,NULL,-1);
	    CommandHarvest(unit,x,y,status);
	    break;
	case MessageCommandMine:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("mine",unit,status,-1,-1,dest,NULL,-1);
	    CommandMineGold(unit,dest,status);
	    break;
	case MessageCommandHaul:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("haul",unit,status,-1,-1,dest,NULL,-1);
	    CommandHaulOil(unit,dest,status);
	    break;
	case MessageCommandReturn:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("return",unit,status,-1,-1,dest,NULL,-1);
	    CommandReturnGoods(unit,dest,status);
	    break;
	case MessageCommandTrain:
	    CommandLog("train",unit,status,-1,-1,NULL
		    ,UnitTypes[dstnr].Ident,-1);
	    CommandTrainUnit(unit,UnitTypes+dstnr,status);
	    break;
	case MessageCommandCancelTrain:
	    CommandLog("cancel-train",unit,FlushCommands,-1,-1,NoUnitP,NULL,x);
	    CommandCancelTraining(unit,x);
	    break;
	case MessageCommandUpgrade:
	    CommandLog("upgrade-to",unit,status,-1,-1,NULL
		    ,UnitTypes[dstnr].Ident,-1);
	    CommandUpgradeTo(unit,UnitTypes+dstnr,status);
	    break;
	case MessageCommandCancelUpgrade:
	    CommandLog("cancel-upgrade-to",unit,FlushCommands,-1,-1,NoUnitP
		    ,NULL,-1);
	    CommandCancelUpgradeTo(unit);
	    break;
	case MessageCommandResearch:
	    CommandLog("research",unit,status,-1,-1,NULL
		    ,Upgrades[x].Ident,-1);
	    CommandResearch(unit,Upgrades+x,status);
	    break;
	case MessageCommandCancelResearch:
	    CommandLog("cancel-research",unit,FlushCommands,-1,-1,NoUnitP
		    ,NULL,-1);
	    CommandCancelResearch(unit);
	    break;
	case MessageCommandDemolish:
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandLog("demolish",unit,status,x,y,dest,NULL,-1);
	    CommandDemolish(unit,x,y,dest,status);
	    break;
	default:
	    id = (msgnr&0x7f) - MessageCommandSpellCast;
	    dest=NoUnitP;
	    if( dstnr!=(unsigned short)0xFFFF ) {
		dest=UnitSlots[dstnr];
		DebugCheck( !dest || !dest->Type );
	    }
	    CommandSpellCast(unit,x,y,dest,SpellTypeById(id),status);
	    break;
    }
}

//@}

//@}
