//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name map_wood.c	-	The map wood handling. */
//
//	(c) Copyright 1999-2002 by Vladi Shabanski and Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*
**	Note:
**		This functions are doubled. One for the real map tile and one
**		for the tile that the player sees.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#ifdef HIERARCHIC_PATHFINDER
#include "pathfinder.h"
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int ForestRegeneration;		/// Forest regeneration

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the seen tile-type is wood.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global int MapIsSeenTileWood(int x, int y)
{
    return TheMap.Tileset->TileTypeTable[
	    TheMap.Fields[(x)+(y)*TheMap.Width].SeenTile
	] == TileTypeWood;
}

/**
**	Correct the seen wood field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenWoodTile(int x, int y)
{
    int tile;
    int ttup;
    int ttdown;
    int ttleft;
    int ttright;
    MapField *mf;
    

    //  Outside of map or no wood.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }

    if (!MapIsSeenTileWood(x,y)) {
	return;
    }
    //
    //  Find out what each tile has with respect to wood, or grass.
    //
    tile = 0;
    ttup = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + (y - 1) * TheMap.Width].SeenTile];
    ttright = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + 1 + y  * TheMap.Width].SeenTile];
    ttdown = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + (y + 1) * TheMap.Width].SeenTile];
    ttleft = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x -1 + y * TheMap.Width].SeenTile];

    //
    //  Check each of the corners to ensure it has both connecting
    //  ?**?
    //  *mm*
    //  *mm*
    //  ?**?
    //  *
    //   *  type asterixs must match for wood to be present
    
    tile += ((ttup & 0x01) && (ttleft & 0x04)) * 8;
    tile += ((ttup & 0x02) && (ttright & 0x08)) * 4;
    tile += ((ttright & 0x01) && (ttdown & 0x04)) * 2;
    tile += ((ttleft & 0x02) && (ttdown & 0x08)) * 1;

    
    tile = TheMap.Tileset->WoodTable[tile];
    //If tile is -1, then we should check if we are to draw just one tree
    //Check for tile about, or below or both...
    if (tile == -1) {
	tile = 16;
	tile += ((ttup & 0x01) || (ttup & 0x02)) * 1;
	tile += ((ttdown & 0x04) || (ttdown & 0x08)) * 2;
	tile = TheMap.Tileset->WoodTable[tile];
    }

    //Update seen tile.
    mf = TheMap.Fields + x + y * TheMap.Width;
    if (tile == -1) {			// No valid wood remove it.
	mf->SeenTile = TheMap.Tileset->RemovedTree;
	MapFixSeenWoodNeighbors(x, y);
    } else if( mf->SeenTile==tile ) {	// Already there!
	return;
    } else {
	mf->SeenTile = tile;
    }

    // FIXME: can this only happen if seen?
#ifdef NEW_FOW
    if (mf->Visible[ThisPlayer->Player]>1) {
#else
    if ( IsMapFieldVisible(x,y) ) {
#endif
	UpdateMinimapSeenXY(x, y);
	MarkDrawPosMap(x, y);
	MustRedraw |= RedrawMinimap;
    }
}

/**
**	Correct the surrounding seen wood fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenWoodNeighbors(int x, int y)
{
    MapFixSeenWoodTile(x + 1, y);		// side neighbors
    MapFixSeenWoodTile(x - 1, y);
    MapFixSeenWoodTile(x, y + 1);
    MapFixSeenWoodTile(x, y - 1);
}

/**
**	Correct the surrounding real wood fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
local void MapFixWoodNeighbors(int x, int y)
{
    MapFixWoodTile(x + 1, y);		// side neighbors
    MapFixWoodTile(x - 1, y);
    MapFixWoodTile(x, y + 1);
    MapFixWoodTile(x, y - 1);
}

/**
**	Correct the real wood field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixWoodTile(int x, int y)
{
    int tile;
    int ttup;
    int ttdown;
    int ttleft;
    int ttright;
    MapField *mf;
    

    //  Outside of map or no wood.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }
    mf = TheMap.Fields + x + y * TheMap.Width;
    if (!(mf->Flags & MapFieldForest)) {
	return;
    }
    //
    //  Find out what each tile has with respect to wood, or grass.
    //
    tile = 0;
    ttup = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + (y - 1) * TheMap.Width].Tile];
    ttright = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + 1 + y  * TheMap.Width].Tile];
    ttdown = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x + (y + 1) * TheMap.Width].Tile];
    ttleft = TheMap.Tileset->MixedLookupTable[
	TheMap.Fields[x -1 + y * TheMap.Width].Tile];

    //
    //  Check each of the corners to ensure it has both connecting
    //  ?**?
    //  *mm*
    //  *mm*
    //  ?**?
    //  *
    //   *  type asterixs must match for wood to be present
    
    tile += ((ttup & 0x01) && (ttleft & 0x04)) * 8;
    tile += ((ttup & 0x02) && (ttright & 0x08)) * 4;
    tile += ((ttright & 0x01) && (ttdown & 0x04)) * 2;
    tile += ((ttleft & 0x02) && (ttdown & 0x08)) * 1;

    
    tile = TheMap.Tileset->WoodTable[tile];
    //If tile is -1, then we should check if we are to draw just one tree
    //Check for tile about, or below or both...
    if (tile == -1) {
	tile = 16;
	tile += ((ttup & 0x01) || (ttup & 0x02)) * 1;
	tile += ((ttdown & 0x04) || (ttdown & 0x08)) * 2;
	tile = TheMap.Tileset->WoodTable[tile];
    }

    if (tile == -1) {			// No valid wood remove it.
	MapRemoveWood(x, y);
    } else if (mf->Tile != tile) {
	mf->Tile = tile;
	UpdateMinimapXY(x, y);
	MapFixWoodNeighbors(x, y);
#ifdef NEW_FOW
	if (mf->Visible[ThisPlayer->Player]>1) {
#else
	if ( IsMapFieldVisible(x,y) ) {
#endif
	    UpdateMinimapSeenXY(x, y);
	    MapMarkSeenTile(x, y);
	    MarkDrawPosMap(x, y);
	    MustRedraw |= RedrawMinimap;
	}
    }
}

/**
**	Remove wood from the map.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapRemoveWood(unsigned x, unsigned y)
{
    MapField *mf;

    mf = TheMap.Fields + x + y * TheMap.Width;

    mf->Tile = TheMap.Tileset->RemovedTree;
    mf->Flags &= ~(MapFieldForest | MapFieldUnpassable);
    mf->Value = 0;

    UpdateMinimapXY(x, y);
    MapFixWoodNeighbors(x, y);

#ifdef NEW_FOW
    if (mf->Visible[ThisPlayer->Player]>1) {
#else
    if ( IsMapFieldVisible(x,y) ) {
#endif
	UpdateMinimapSeenXY(x, y);
	MapMarkSeenTile(x, y);
	MarkDrawPosMap(x, y);
	MustRedraw |= RedrawMinimap;
    }
#ifdef HIERARCHIC_PATHFINDER
    PfHierMapChangedCallback (x, y, x, y);
#endif
}

/**
**	Regenerate forest.
*/
global void RegenerateForest(void)
{
    MapField *mf;
    MapField *tmp;
    int x;
    int y;

    if (!ForestRegeneration) {
	return;
    }

    //
    //  Increment each value of no wood.
    //  If gown up, place new wood.
    //  FIXME: a better looking result would be fine
    for (x = 0; x < TheMap.Width; ++x) {
	for (y = 0; y < TheMap.Height; ++y) {
	    mf = TheMap.Fields + x + y * TheMap.Width;
	    if (mf->Tile == TheMap.Tileset->RemovedTree) {
		if (mf->Value >= ForestRegeneration
		    || ++mf->Value == ForestRegeneration) {
		    if (x
			&& !(mf->
			     Flags & (MapFieldWall | MapFieldUnpassable |
				      MapFieldLandUnit | MapFieldBuilding))) {
			tmp = mf - TheMap.Width;
			if (tmp->Tile == TheMap.Tileset->RemovedTree
			    && tmp->Value >= ForestRegeneration
			    && !(tmp->
				 Flags & (MapFieldWall | MapFieldUnpassable |
					  MapFieldLandUnit |
					  MapFieldBuilding))) {

			    DebugLevel0("Real place wood\n");
			    tmp->Tile = TheMap.Tileset->TopOneTree;
			    tmp->Value = 0;
			    tmp->Flags |= MapFieldForest | MapFieldUnpassable;

			    mf->Tile = TheMap.Tileset->BotOneTree;
			    mf->Value = 0;
			    mf->Flags |= MapFieldForest | MapFieldUnpassable;
#ifdef NEW_FOW
			    if (mf->Visible[ThisPlayer->Player]>1) {
				MapMarkSeenTile(x, y);
			    }
			    if (tmp->Visible[ThisPlayer->Player]>1) {
				MapMarkSeenTile(x, y-1);
			    }
#else
			    if ( IsMapFieldVisible(x,y) ) {
				MapMarkSeenTile(x, y);
			    }
			    if ( IsMapFieldVisible(x,y-1) ) {
				MapMarkSeenTile(x, y-1);
			    }
#endif
			}
		    }
		}
	    }
	}
    }
}

//@}
