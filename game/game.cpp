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
/**@name game.c		-	The game set-up and creation. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer, Andreas Arens
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "freecraft.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "pud.h"
#include "ui.h"
#include "font.h"
#include "sound_server.h"
#include "menus.h"
#include "depend.h"
#include "interface.h"
#include "cursor.h"
#include "image.h"
#include "spells.h"
#include "construct.h"
#include "network.h"
#include "settings.h"

#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global Settings GameSettings;		/// Game Settings
global int lcm_prevent_recurse = 0;	/// prevent recursion through LoadGameMap

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Map loading/saving
----------------------------------------------------------------------------*/

/**
**	Load a clone map.
**
**	@param filename	map filename
**	@param map	map loaded
*/
local void LoadCloneMap(const char* filename,WorldMap* map)
{
    DebugLevel3(__FUNCTION__" %p \n",map);

#if defined(USE_CCL) || defined(USE_CCL2)
    if (lcm_prevent_recurse) {
	fprintf(stderr,"recursive use of load clone map!\n");
	exit(-1);
    }
    lcm_prevent_recurse = 1;
    gh_eval_file((char*)filename);
    lcm_prevent_recurse = 0;
    if (!ThisPlayer) {		/// ARI: bomb if nothing was loaded!
	fprintf(stderr,"%s: invalid clone map\n", filename);
	exit(-1);
    }
#else
    fprintf(stderr,"Sorry, you need guile/siod installed to use clone maps!\n");
    exit(-1);
#endif
}

/**
**	Load any map.
**
**	@param filename	map filename
**	@param map	map loaded
*/
global void LoadMap(const char* filename,WorldMap* map)
{
    const char* tmp;

    tmp=strrchr(filename,'.');
    if( tmp ) {
#ifdef USE_ZLIB
	if( !strcmp(tmp,".gz") ) {
	    while( tmp-1>filename && *--tmp!='.' ) {
	    }
	} else
#endif
#ifdef USE_BZ2LIB
	if( !strcmp(tmp,".bz2") ) {
	    while( tmp-1>filename && *--tmp!='.' ) {
	    }
	}
#endif
	if( !strcmp(tmp,".cm")
#ifdef USE_ZLIB
		|| !strcmp(tmp,".cm.gz")
#endif
#ifdef USE_BZ2LIB
		|| !strcmp(tmp,".cm.bz2")
#endif
	) {
	    LoadCloneMap(filename,map);
	    return;
	}
    }
    // ARI: This bombs out, if no pud, so will be safe.
    LoadPud(filename,map);
}

/**
**	Save a map.
**
**	@param file	Output file.
*/
global void SaveMap(FILE* file)
{
    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: map $Id$\n");

    fprintf(file,"(freecraft-map\n");

    // FIXME: Need version number here!
    fprintf(file,"  '(version %d.%d)\n",0,0);
    fprintf(file,"  '(description \"Saved\")\n");
    fprintf(file,"  '(terrain %d \"%s\")\n"
	    ,TheMap.Terrain,Tilesets[TheMap.Terrain].Name);
    fprintf(file,"  '(tiles #(\n");
    fprintf(file,"  )\n");

    fprintf(file,")\n");

    fprintf(file,"(the-map\n");
    fprintf(file,"  (%d %d)\n",TheMap.Width,TheMap.Height);
    fprintf(file,"  %d\n",TheMap.NoFogOfWar);
    fprintf(file,")\n");
}

/*----------------------------------------------------------------------------
--	Game creation
----------------------------------------------------------------------------*/

/**
**	CreateGame.
**
**	Load map, graphics, sounds, etc
**
**	@param filename	map filename
**	@param map	map loaded
*/
global void CreateGame(char* filename, WorldMap* map)
{
    char* PalettePath;
    int i, j;

    if (filename == NULL) {
	ProcessMenu(MENU_PRG_START, 1);
    } else {
	if (filename[0] != '/' && filename[0] != '.') {
	    filename = strdcat3(FreeCraftLibPath, "/", filename);
	}
	//
	//	Load the map.
	//
	LoadMap(filename, map);
    }

    if( FlagRevealMap ) {
	RevealMap();
    }

    if (GameSettings.Resources != SettingsResourcesMapDefault) {
	for (j = 0; j < PlayerMax; ++j) {
	    if (Players[j].Type == PlayerNobody) {
		continue;
	    }
	    for (i = 1; i < MaxCosts; ++i) {
		switch (GameSettings.Resources) {
		    case SettingsResourcesLow:
			Players[j].Resources[i]=DEFAULT_RESOURCES_LOW[i];
			break;
		    case SettingsResourcesMedium:
			Players[j].Resources[i]=DEFAULT_RESOURCES_MEDIUM[i];
			break;
		    case SettingsResourcesHigh:
			Players[j].Resources[i]=DEFAULT_RESOURCES_HIGH[i];
			break;
		    default:
			break;
		}
	    }
	}
    }

    DebugLevel3("Terrain %d\n",TheMap.Terrain);

    // FIXME: must use palette from tileset!!
    // FIXME: this must be extendable!!

    switch( TheMap.Terrain ) {
	case TilesetSummer:
            PalettePath = strdcat(FreeCraftLibPath, "/summer.rgb");
	    break;
	case TilesetWinter:
            PalettePath = strdcat(FreeCraftLibPath, "/winter.rgb");
	    break;
	case TilesetWasteland:
            PalettePath = strdcat(FreeCraftLibPath, "/wasteland.rgb");
	    break;
	case TilesetSwamp:
            PalettePath = strdcat(FreeCraftLibPath, "/swamp.rgb");
	    break;
	default:
	    DebugLevel2("Unknown Terrain %d\n",TheMap.Terrain);
            PalettePath = strdcat(FreeCraftLibPath, "/summer.rgb");
	    break;
    }
    LoadRGB(GlobalPalette, PalettePath);
    VideoCreatePalette(GlobalPalette);

    //
    //	Graphic part
    //
    LoadIcons();
    InitMenus(ThisPlayer->Race);
    LoadImages(ThisPlayer->Race);
    LoadCursors(ThisPlayer->Race);
    LoadTileset();
    InitUnitButtons();
    LoadMissileSprites();
    InitSpells();
    LoadUnitSprites();
    LoadConstructions();
    LoadDecorations();

    IfDebug(
	DebugLevel0("Graphics uses %d bytes (%d KB, %d MB)\n"
		,AllocatedGraphicMemory
		,AllocatedGraphicMemory/1024
		,AllocatedGraphicMemory/1024/1024);
	DebugLevel0("Compressed graphics uses %d bytes (%d KB, %d MB)\n"
		,CompressedGraphicMemory
		,CompressedGraphicMemory/1024
		,CompressedGraphicMemory/1024/1024);
    );

    CreateMinimap();			// create minimap for pud
    InitMap();				// setup draw functions
    InitMapFogOfWar();			// build tables for fog of war
    PreprocessMap();			// Adjust map for use
    MapColorCycle();			// Setup color cycle

    InitUserInterface();		// Setup the user interface.

    //
    //	Sound part
    //
    //FIXME: check if everything is really loaded
    LoadUnitSounds();
    MapUnitSounds();

#ifdef WITH_SOUND
    IfDebug(
	DebugLevel0("Sounds uses %d bytes (%d KB, %d MB)\n"
		,AllocatedSoundMemory
		,AllocatedSoundMemory/1024
		,AllocatedSoundMemory/1024/1024);
	DebugLevel0("Compressed sounds uses %d bytes (%d KB, %d MB)\n"
		,CompressedSoundMemory
		,CompressedSoundMemory/1024
		,CompressedSoundMemory/1024/1024);
    );
#endif

    //
    //	Network part
    //
    if( NetPlayers>1 || NetworkArg ) {	// with network
	InitNetwork1();
	InitNetwork2();
    } else {
	NetworkFildes=-1;
	NetworkInSync=1;
    }

    //
    //  Init units' groups
    //
    InitGroups();

    //
    //	Init players?
    //
    DebugPlayers();
    PlayersInitAi();

    //
    //  Upgrades
    //
    InitUpgrades();

    //
    //  Dependencies
    //
    InitDependencies();

    //
    //  Buttons (botpanel)
    //
    InitButtons();

#ifdef WITH_SOUND
    if (SoundFildes!=-1) {
	//FIXME: must be done after map is loaded
	if ( InitSoundServer() ) {
	    SoundOff=1;
	    SoundFildes=-1;
	} else {
	    // must be done after sounds are loaded
	    InitSoundClient();
	}
    }
#endif

    if( ThisPlayer->Race==PlayerRaceHuman ) {
	SetDefaultTextColors(FontWhite,FontYellow);
	// FIXME: Add this again:
	//	PlaySound(SoundBasicHumanVoicesSelected1);
    } else if( ThisPlayer->Race==PlayerRaceOrc ) {
	SetDefaultTextColors(FontYellow,FontWhite);
	// FIXME: Add this again:
	//	PlaySound(SoundBasicOrcVoicesSelected1);
    }
    // FIXME: support more races

    MapCenter(ThisPlayer->X,ThisPlayer->Y);

    //FIXME: must be done after map is loaded
    if(AStarOn) {
	InitAStar();
    }

}


/**
**	Init Game Setting to default values
**
*/
global void InitSettings(void)
{
    int i;

    for (i = 0; i < PlayerMax; i++) {
	GameSettings.Presets[i].Race = SettingsPresetMapDefault;
	GameSettings.Presets[i].Team = SettingsPresetMapDefault;
    }
    GameSettings.Resources = SettingsResourcesMapDefault;
    GameSettings.NumUnits = SettingsNumUnitsMapDefault;
    GameSettings.Opponents = SettingsPresetMapDefault;
    GameSettings.Terrain = SettingsPresetMapDefault;
}

//@}
