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
/**@name action_train.c -	The building train action. */
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "missile.h"
#include "sound.h"
#include "ai.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit trains unit!
**
**	@param unit	Unit that trains.
*/
global void HandleActionTrain(Unit* unit)
{
    Unit* nunit;
    const UnitType* type;
    Player* player;

    player=unit->Player;
#ifdef NEW_ORDERS
    unit->Data.Train.Ticks+=SpeedTrain;
    // FIXME: Should count down
    if( unit->Data.Train.Ticks
	    >=unit->Data.Train.What[0]
		->Stats[player->Player].Costs[TimeCost] ) {
#else
    unit->Command.Data.Train.Ticks+=SpeedTrain;
    // FIXME: Should count down
    if( unit->Command.Data.Train.Ticks
	    >=unit->Command.Data.Train.What[0]
		->Stats[player->Player].Costs[TimeCost] ) {
#endif

	//
	//	Check if enough food available.
	//
	if( player->Food<=player->NumFoodUnits ) {

	    // FIXME: GameMessage
	    if( player==ThisPlayer ) {
		// FIXME: PlayVoice :), see task.txt
		SetMessage("You need more farms!");
	    } else {
		// FIXME: Callback for AI!
		// AiNeedMoreFarms(unit);
	    }

#ifdef NEW_ORDERS
	    unit->Data.Train.Ticks-=SpeedTrain;
#else
	    unit->Command.Data.Train.Ticks-=SpeedTrain;
#endif
	    unit->Reset=1;
	    unit->Wait=FRAMES_PER_SECOND/6;
	    return;
	}

#ifdef NEW_ORDERS
	nunit=MakeUnit(unit->Data.Train.What[0],player);
#else
	nunit=MakeUnit(unit->Command.Data.Train.What[0],player);
#endif
	nunit->X=unit->X;
	nunit->Y=unit->Y;
	type=unit->Type;
	DropOutOnSide(nunit,LookingW,type->TileWidth,type->TileHeight);

	// FIXME: GameMessage
	if( player==ThisPlayer ) {
	    SetMessage2( nunit->X, nunit->Y, "New %s ready", nunit->Type->Name);
	    PlayUnitSound(nunit,VoiceReady);
	} else {
	    AiTrainingComplete(unit,nunit);
	}

	unit->Reset=1;
	unit->Wait=1;
        
#ifdef NEW_ORDERS
	if ( --unit->Data.Train.Count ) {
	    int z;
	    for( z = 0; z < unit->Data.Train.Count ; z++ ) {
		unit->Data.Train.What[z]=unit->Data.Train.What[z+1];
	    }
	    unit->Data.Train.Ticks=0;
	} else {
	    unit->Orders[0].Action=UnitActionStill;
	}

	nunit->Orders[0]=unit->NewOrder;

	//
	// FIXME: Pending command uses any references?
	//
	if( nunit->Orders[0].Goal ) {
	    if( nunit->Orders[0].Goal->Destroyed ) {
		DebugLevel0Fn("FIXME: you have found a bug, please fix it.\n");
	    }
	    RefsDebugCheck( !nunit->Orders[0].Goal->Refs );
	    nunit->Orders[0].Goal->Refs++;
	}
#else
	if ( --unit->Command.Data.Train.Count ) {
	    int z;
	    for( z = 0; z < unit->Command.Data.Train.Count ; z++ ) {
		unit->Command.Data.Train.What[z] =
			unit->Command.Data.Train.What[z+1];
	    }
	    unit->Command.Data.Train.Ticks=0;
	} else {
	    unit->Command.Action=UnitActionStill;
	}

	nunit->Command=unit->PendCommand;
	//
	// FIXME: Pending command uses any references?
	//
	if( nunit->Command.Data.Move.Goal ) {
	    if( nunit->Command.Data.Move.Goal->Destroyed ) {
		DebugLevel0Fn("FIXME: you have found a bug, please fix it.\n");
	    }
	    nunit->Command.Data.Move.Goal->Refs++;
	}
#endif

	if( IsSelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}

	return;
    }

    if( IsSelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=FRAMES_PER_SECOND/6;
}

//@}
