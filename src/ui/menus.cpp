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
/**@name menus.c	-	The menu function code. */
//
//	(c) Copyright 1999-2002 by Andreas Arens
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

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#include "freecraft.h"

#include "iocompat.h"

#include "video.h"
#include "player.h"
#include "font.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "menus.h"
#include "cursor.h"
#include "pud.h"
#include "iolib.h"
#include "network.h"
#include "netconnect.h"
#include "settings.h"
#include "ui.h"
#include "campaign.h"
#include "sound_server.h"
#include "sound.h"
#include "ccl.h"
#include "editor.h"

#if defined(USE_SDLCD) || defined(USE_SDLA)
#include "SDL.h"
#endif

#ifdef USE_LIBCDA
#include "libcda.h"
#endif

//#define OLD_MENU		/// CCL menus not used
//#define SAVE_MENU_CCL		/// SAVE (REWRITE!) the menus.ccl file

/*----------------------------------------------------------------------------
--	Prototypes for local functions
----------------------------------------------------------------------------*/

local void EditorEndMenu(void);

/*----------------------------------------------------------------------------
--	Prototypes for action handlers and helper functions
----------------------------------------------------------------------------*/

local void GameMenuEnd(void);
local void GameMenuExit(void);
local void GameMenuReturn(void);
local void GameShowCredits(void);
local void GameMenuObjectives(void);
local void GameMenuEndScenario(void);
local void GameOptions(void);

local void HelpMenu(void);
local void KeystrokeHelpMenu(void);
local void ShowTipsMenu(void);
local void InitTips(Menuitem *mi);
local void TipsMenuEnd(void);
local void SetTips(Menuitem *mi);
local void ShowNextTip(void);

local void InitGameMenu(Menuitem *mi);

local void GlobalOptions(void);
local void InitGlobalOptions(Menuitem *mi);
local void SetRes(Menuitem *mi);
local void SetFullscreen(Menuitem *mi);
local void SetShadowFogAlpha(Menuitem *mi);
local void SetShadowFogGray(Menuitem *mi);

local void InitSoundOptions(Menuitem *mi);
local void SetMasterPower(Menuitem *mi);
local void SetMusicPower(Menuitem *mi);
local void SetCdPower(Menuitem *mi);
local void SetFogOfWar(Menuitem *mi);
local void SetCommandKey(Menuitem *mi);
local void SetCdModeAll(Menuitem *mi);
local void SetCdModeRandom(Menuitem *mi);

local void EndScenarioRestart(void);
local void EndScenarioSurrender(void);
local void EndScenarioQuitMenu(void);

local void PrgStartInit(Menuitem *mi);		// master init
local void NameLineDrawFunc(Menuitem *mi);
local void EnterNameAction(Menuitem *mi, int key);
local void EnterNameCancel(void);
local void EnterServerIPAction(Menuitem *mi, int key);
local void EnterServerIPCancel(void);

local void InitSaveGameMenu(Menuitem *mi);
local void EnterSaveGameAction(Menuitem *mi, int key);
local void SaveAction(void);
local void CreateSaveDir(void);

local void SaveLBExit(Menuitem *mi);
local void SaveLBInit(Menuitem *mi);
local unsigned char *SaveLBRetrieve(Menuitem *mi, int i);
local void SaveLBAction(Menuitem *mi, int i);
local void SaveVSAction(Menuitem *mi, int i);
local void SaveOk(void);

local void InitLoadGameMenu(Menuitem *mi);
local void LoadLBExit(Menuitem *mi);
local void LoadLBInit(Menuitem *mi);
local unsigned char *LoadLBRetrieve(Menuitem *mi, int i);
local void LoadLBAction(Menuitem *mi, int i);
local void LoadVSAction(Menuitem *mi, int i);
local void LoadOk(void);
local int SaveRDFilter(char *pathbuf, FileList *fl);

local void FcDeleteMenu(void);
local void FcDeleteInit(Menuitem *mi);
local void FcDeleteFile(void);

//local void ConfirmSaveMenu(void);
local void ConfirmSaveInit(Menuitem *mi);
local void ConfirmSaveFile(void);

local void LoadAction(void);

local void JoinNetGameMenu(void);
local void CreateNetGameMenu(void);

local void SinglePlayerGameMenu(void);
local void MultiPlayerGameMenu(void);
local void CampaignGameMenu(void);
local void ScenSelectMenu(void);

local void CampaignMenu1(void);
local void CampaignMenu2(void);
local void CampaignMenu3(void);
local void CampaignMenu4(void);
local void SelectCampaignMenu(void);

local void ScenSelectLBExit(Menuitem *mi);
local void ScenSelectLBInit(Menuitem *mi);
local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i);
local void ScenSelectLBAction(Menuitem *mi, int i);
local void ScenSelectTPMSAction(Menuitem *mi, int i);
local void ScenSelectVSAction(Menuitem *mi, int i);
local void ScenSelectFolder(void);
local void ScenSelectInit(Menuitem *mi);	// master init
local void ScenSelectOk(void);
local void ScenSelectCancel(void);
local int ScenSelectRDFilter(char *pathbuf, FileList *fl);

local void KeystrokeHelpVSAction(Menuitem *mi, int i);
local void KeystrokeHelpDrawFunc(Menuitem *mi);

local void GameSpeedHSAction(Menuitem *mi, int i);
local void MouseScrollHSAction(Menuitem *mi, int i);
local void KeyboardScrollHSAction(Menuitem *mi, int i);
local void MasterVolumeHSAction(Menuitem *mi, int i);
local void MusicVolumeHSAction(Menuitem *mi, int i);
local void CdVolumeHSAction(Menuitem *mi, int i);

local void GameSetupInit(Menuitem *mi);		// master init
local void GameDrawFunc(Menuitem *mi);

local void GameRCSAction(Menuitem *mi, int i);
local void GameRESAction(Menuitem *mi, int i);
local void GameUNSAction(Menuitem *mi, int i);
local void GameTSSAction(Menuitem *mi, int i);
local void GameGATAction(Menuitem *mi, int i);

local void GameCancel(void);

local void CustomGameStart(void);
local void CustomGameOPSAction(Menuitem *mi, int i);

local void MultiClientReady(void);
local void MultiClientNotReady(void);
local void MultiClientGemAction(Menuitem *mi);
local void MultiClientCancel(void);
local void MultiClientRCSAction(Menuitem *mi, int i);

local void MultiGameStart(void);
local void MultiGameCancel(void);
local void NetworkGamePrepareGameSettings(void);
local void MultiGameSetupInit(Menuitem *mi);	// master init
local void MultiGameSetupExit(Menuitem *mi);	// master exit
local void MultiGameDrawFunc(Menuitem *mi);
local void MultiGameFWSAction(Menuitem *mi, int i);
local void MultiGamePTSAction(Menuitem *mi, int o);
local void NetMultiPlayerDrawFunc(Menuitem *mi);
local void MultiGamePlayerSelectorsUpdate(int initial);
local void MultiScenSelectMenu(void);

local void MultiGameClientInit(Menuitem *mi);	// master init
local void MultiGameClientDrawFunc(Menuitem *mi);
local void MultiClientUpdate(int initial);

local void NetConnectingCancel(void);
local void TerminateNetConnect(void);

local void StartEditor(void);
local void EditorNewMap(void);
local void EditorLoadMap(void);
local void EditorLoadInit(Menuitem *mi);
local void EditorLoadLBInit(Menuitem *mi);
local void EditorLoadLBExit(Menuitem *mi);
local void EditorLoadFolder(void);
local int EditorLoadRDFilter(char *pathbuf, FileList *fl);
local void EditorLoadLBAction(Menuitem *mi, int i);
local unsigned char *EditorLoadLBRetrieve(Menuitem *mi, int i);
local void EditorLoadOk(void);
local void EditorLoadCancel(void);
local void EditorLoadVSAction(Menuitem *mi, int i);
local void EditorMapProperties(void);
local void EditorPlayerProperties(void);
local void EditorPlayerPropertiesDrawFunc(Menuitem *mi);
local void EditorPlayerPropertiesEnterAction(Menuitem *mi, int key);
local void EditorEnterMapDescriptionAction(Menuitem *mi, int key);
local void EditorMapPropertiesOk(void);
local void EditorEditResourceEnterAction(Menuitem *mi,int key);
local void EditorEditResourceOk(void);
local void EditorEditResourceCancel(void);
local void EditorQuitMenu(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// FIXME: Docu, Johns why typedef?
global _MenuHash MenuHash;
    /// FIXME: Docu, Johns why typedef?
global _MenuFuncHash MenuFuncHash;

#define HASHADD(x,y) { \
    *(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
}

    /// Name, Version, Copyright FIXME: move to headerfile
extern char NameLine[];

local int GameLoaded;
local int GuiGameStarted;
local int EditorLoadCancelled;

/**
**	Other client and server selection state for Multiplayer clients
*/
global ServerSetup ServerSetupState, LocalSetupState;

local char ScenSelectPath[1024];		/// Scenario selector path
local char ScenSelectDisplayPath[1024];		/// Displayed selector path
local char ScenSelectFileName[128];		/// Scenario selector name
global char ScenSelectFullPath[1024];		/// Scenario selector path+name

global MapInfo *ScenSelectPudInfo;		/// Selected pud info

global int nKeyStrokeHelps;
global char **KeyStrokeHelps;

/// FIXME: -> ccl...
local unsigned char *mgptsoptions[] = {
    "Available",
    "Computer",
    "Closed",
};

/**
**	Help-items for the Net Multiplayer Setup and Client Menus
*/
local Menuitem NetMultiButtonStorage[] = {
    { MI_TYPE_PULLDOWN, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
    { MI_TYPE_DRAWFUNC, 40, 32, 0, GameFont, NULL, NULL, NULL, {{NULL,0}} },
};
local void InitNetMultiButtonStorage() {
    MenuitemPulldown i0 = { mgptsoptions, 172, 20, MBUTTON_PULLDOWN, MultiGamePTSAction, 3, -1, 0, 0, 0};
    MenuitemDrawfunc i1 = { NetMultiPlayerDrawFunc };
    NetMultiButtonStorage[0].d.pulldown = i0;
    NetMultiButtonStorage[1].d.drawfunc = i1;
}

#include "menu_defs.inc"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

global void InitMenuFuncHash(void) {
#ifdef SAVE_MENU_CCL
    char buf[10];
#endif

    HASHADD(NULL,"null");

// Game menu
    HASHADD(InitGameMenu,"init-game-menu");
    HASHADD(GameMenuSave,"game-menu-save");
    HASHADD(GameMenuLoad,"game-menu-load");
    HASHADD(GameOptions,"game-options");
    HASHADD(HelpMenu,"menu-help");
    HASHADD(GameMenuObjectives,"game-menu-objectives");
    HASHADD(GameMenuEndScenario,"game-menu-end-scenario");
    HASHADD(GameMenuReturn,"game-menu-return");
    HASHADD(EndMenu,"end-menu");

// Victory, lost
    HASHADD(GameMenuEnd,"game-menu-end");

// Scene select
    HASHADD(ScenSelectMenu,"scen-select-menu");
    HASHADD(ScenSelectLBExit,"scen-select-lb-exit");
    HASHADD(ScenSelectLBInit,"scen-select-lb-init");
    HASHADD(ScenSelectLBRetrieve,"scen-select-lb-retrieve");
    HASHADD(ScenSelectLBAction,"scen-select-lb-action");
    HASHADD(ScenSelectTPMSAction,"scen-select-tpms-action");
    HASHADD(ScenSelectVSAction,"scen-select-vs-action");
    HASHADD(ScenSelectFolder,"scen-select-folder");
    HASHADD(ScenSelectInit,"scen-select-init");
    HASHADD(ScenSelectOk,"scen-select-ok");
    HASHADD(ScenSelectCancel,"scen-select-cancel");
    HASHADD(ScenSelectRDFilter,"scen-select-rd-filter");

// Program start
    HASHADD(PrgStartInit,"program-start");
    HASHADD(NameLineDrawFunc,"name-line-draw");
    HASHADD(SinglePlayerGameMenu,"single-player-game-menu");
    HASHADD(MultiPlayerGameMenu,"multi-player-game-menu");
    HASHADD(CampaignGameMenu,"campaign-game-menu");
    HASHADD(StartEditor,"game-start-editor");
    HASHADD(GameShowCredits,"game-show-credits");
    HASHADD(GameMenuExit,"game-menu-exit");

// Global Options
    HASHADD(GlobalOptions,"menu-global-options");
    HASHADD(InitGlobalOptions,"init-global-options");
    HASHADD(SetRes,"set-res");
    HASHADD(SetFullscreen,"set-fullscreen");
    HASHADD(SetShadowFogAlpha,"set-shadow-fog-alpha");
    HASHADD(SetShadowFogGray,"set-shadow-fog-gray");

// Tips
    HASHADD(InitTips,"init-tips");
    HASHADD(SetTips,"set-tips");
    HASHADD(ShowNextTip,"show-next-tip");
    HASHADD(TipsMenuEnd,"tips-menu-end");

// Custom game setup
    HASHADD(GameSetupInit,"game-setup-init");
    HASHADD(GameDrawFunc,"game-draw-func");
    HASHADD(GameRCSAction,"game-rcs-action");
    HASHADD(GameRESAction,"game-res-action");
    HASHADD(GameUNSAction,"game-uns-action");
    HASHADD(GameTSSAction,"game-tss-action");
    HASHADD(GameGATAction,"game-gat-action");
    HASHADD(GameCancel,"game-cancel");
    HASHADD(CustomGameStart,"custom-game-start");
    HASHADD(CustomGameOPSAction,"custom-game-ops-action");

// Enter name
    HASHADD(EnterNameAction,"enter-name-action");
    HASHADD(EnterNameCancel,"enter-name-cancel");

// Net create join
    HASHADD(JoinNetGameMenu,"net-join-game");
    HASHADD(CreateNetGameMenu,"net-create-game");

// Net multi setup
    HASHADD(MultiGameSetupInit,"multi-game-setup-init");
    HASHADD(MultiGameDrawFunc,"multi-game-draw-func");
    HASHADD(MultiScenSelectMenu,"multi-scen-select");
    HASHADD(MultiGameStart,"multi-game-start");
    HASHADD(MultiGameCancel,"multi-game-cancel");
    HASHADD(MultiGameFWSAction,"multi-game-fws-action");

// Enter server ip
    HASHADD(EnterServerIPAction,"enter-server-ip-action");
    HASHADD(EnterServerIPCancel,"enter-server-ip-cancel");

// Net multi client
    HASHADD(TerminateNetConnect,"terminate-net-connect");
    HASHADD(MultiGameClientInit,"multi-game-client-init");
    HASHADD(MultiGameClientDrawFunc,"multi-client-draw-func");
    HASHADD(MultiClientReady,"multi-client-ready");
    HASHADD(MultiClientNotReady,"multi-client-not-ready");
    HASHADD(MultiClientCancel,"multi-client-cancel");
    HASHADD(MultiClientRCSAction,"multi-client-rcs-action");
    HASHADD(MultiClientGemAction,"multi-client-gem-action");

// Net connecting
    HASHADD(NetConnectingCancel,"net-connecting-cancel");

// Campaign select
    HASHADD(CampaignMenu1,"campaign-1");
    HASHADD(CampaignMenu2,"campaign-2");
    HASHADD(CampaignMenu3,"campaign-3");
    HASHADD(CampaignMenu4,"campaign-4");
    HASHADD(SelectCampaignMenu,"select-campaign-menu");

// End scenario
    HASHADD(EndScenarioRestart,"end-scenario-restart");
    HASHADD(EndScenarioSurrender,"end-scenario-surrender");
    HASHADD(EndScenarioQuitMenu,"end-scenario-quit-menu");

// Sound options
    HASHADD(MasterVolumeHSAction,"master-volume-hs-action");
    HASHADD(SetMasterPower,"set-master-power");
    HASHADD(MusicVolumeHSAction,"music-volume-hs-action");
    HASHADD(SetMusicPower,"set-music-power");
    HASHADD(CdVolumeHSAction,"cd-volume-hs-action");
    HASHADD(SetCdPower,"set-cd-power");
    HASHADD(SetCdModeAll,"set-cd-mode-all");
    HASHADD(SetCdModeRandom,"set-cd-mode-random");

// Preferences
    HASHADD(SetFogOfWar,"set-fog-of-war");
    HASHADD(SetCommandKey,"set-command-key");

// Speed settings
    HASHADD(GameSpeedHSAction,"game-speed-hs-action");
    HASHADD(MouseScrollHSAction,"mouse-scroll-hs-action");
    HASHADD(KeyboardScrollHSAction,"keyboard-scroll-hs-action");

// Game options
    HASHADD(SoundOptions,"sound-options");
    HASHADD(InitSoundOptions,"init-sound-options");
    HASHADD(SpeedSettings,"speed-settings");
    HASHADD(Preferences,"preferences");

// Help
    HASHADD(KeystrokeHelpMenu,"keystroke-help");
    HASHADD(ShowTipsMenu,"show-tips");

// Keystroke help
    HASHADD(KeystrokeHelpVSAction,"keystroke-help-vs-action");
    HASHADD(KeystrokeHelpDrawFunc,"keystroke-help-draw-func");

// Save
    HASHADD(InitSaveGameMenu,"init-save-game-menu");
    HASHADD(SaveLBInit,"save-lb-init");
    HASHADD(SaveLBExit,"save-lb-exit");
    HASHADD(EnterSaveGameAction,"enter-save-game-action");
    HASHADD(SaveLBAction,"save-lb-action");
    HASHADD(SaveLBRetrieve,"save-lb-retrieve");
    HASHADD(SaveVSAction,"save-vs-action");
    HASHADD(SaveOk,"save-ok");
    HASHADD(SaveAction,"save-action");
    HASHADD(FcDeleteMenu,"fc-delete-menu");

// Load
    HASHADD(InitLoadGameMenu,"init-load-game-menu");
    HASHADD(LoadLBInit,"load-lb-init");
    HASHADD(LoadLBExit,"load-lb-exit");
    HASHADD(LoadLBAction,"load-lb-action");
    HASHADD(LoadLBRetrieve,"load-lb-retrieve");
    HASHADD(LoadVSAction,"load-vs-action");
    HASHADD(LoadOk,"load-ok");
    HASHADD(LoadAction,"load-action");

// Confirm save
    HASHADD(ConfirmSaveInit,"confirm-save-init");
    HASHADD(ConfirmSaveFile,"confirm-save-file");

// Confirm delete
    HASHADD(FcDeleteInit,"fc-delete-init");
    HASHADD(FcDeleteFile,"fc-delete-file");

// Editor select
    HASHADD(EditorNewMap,"editor-new-map");
    HASHADD(EditorLoadMap,"editor-load-map");

// Editor load map
    HASHADD(EditorLoadInit,"editor-load-init");
    HASHADD(EditorLoadLBInit,"editor-load-lb-init");
    HASHADD(EditorLoadLBExit,"editor-load-lb-exit");
    HASHADD(EditorLoadLBAction,"editor-load-lb-action");
    HASHADD(EditorLoadLBRetrieve,"editor-load-lb-retrieve");
    HASHADD(EditorLoadVSAction,"editor-load-vs-action");
    HASHADD(EditorLoadOk,"editor-load-ok");
    HASHADD(EditorLoadCancel,"editor-load-cancel");
    HASHADD(EditorLoadFolder,"editor-load-folder");
    HASHADD(EditorMapProperties,"menu-editor-map-properties");
    HASHADD(EditorEnterMapDescriptionAction,"editor-enter-map-description-action");
    HASHADD(EditorPlayerProperties,"menu-editor-player-properties");

// Editor menu
    HASHADD(EditorQuitMenu,"editor-quit-menu");

// Editor map properties
    HASHADD(EditorEnterMapDescriptionAction,"editor-map-description-action");
    HASHADD(EditorMapPropertiesOk,"editor-map-properties-ok");
    HASHADD(EditorEndMenu,"editor-end-menu");

// Editor player properties
    HASHADD(EditorPlayerPropertiesDrawFunc,"editor-player-properties-draw-func");
    HASHADD(EditorPlayerPropertiesEnterAction,"editor-player-properties-enter-action");

// Editor edit resource
    HASHADD(EditorEditResourceEnterAction,"editor-edit-resource-enter-action");
    HASHADD(EditorEditResourceOk,"editor-edit-resource-ok");
    HASHADD(EditorEditResourceCancel,"editor-edit-resource-cancel");
}

/*----------------------------------------------------------------------------
--	Button action handler and Init/Exit functions
----------------------------------------------------------------------------*/

/**
**	Draw the version and copyright at bottom of the screen.
**	Also include now the license.
*/
local void NameLineDrawFunc(Menuitem * mi __attribute__ ((unused)))
{
    int nc, rc;

    GetDefaultTextColors(&nc, &rc);
    MenusSetBackground();
    SetDefaultTextColors(rc, rc);

#ifdef WITH_SOUND
    if (SoundFildes == -1) {
	VideoDrawText(16, 16, LargeFont, "Sound disabled, please check!");
    }
#endif

    VideoDrawTextCentered(VideoWidth/2, TheUI.Offset480Y + 440, GameFont, NameLine);
    VideoDrawTextCentered(VideoWidth/2, TheUI.Offset480Y + 456, GameFont,
	"Engine distributed under the terms of the GNU General Public License.");
    SetDefaultTextColors(nc, rc);
}

/**
**	Start menu master init.
**
**	@param mi	The menu.
*/
local void PrgStartInit(Menuitem *mi)
{
    if (NetworkNumInterfaces == 0) {
	mi[2].flags = MenuButtonDisabled;
    } else {
	mi[2].flags = 0;
    }
}

local void GameMenuReturn(void)
{
    EndMenu();
    MustRedraw &= ~RedrawMenu;
    InterfaceState = IfaceStateNormal;
    ClearStatusLine();
    MarkDrawEntireMap();	//FIXME: some tiles could be left as they were?
    MustRedraw=RedrawEverything;
    GamePaused = 0;
}

local char *SaveDir;
local int TypedFileName;

local void InitSaveGameMenu(Menuitem *mi)
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    menu->items[4].flags = MenuButtonDisabled;    
    menu->items[5].flags = MenuButtonDisabled;
    CreateSaveDir();
}

/**
**	FIXME: docu.
*/
local void EnterSaveGameAction(Menuitem *mi, int key)
{
#ifdef OLD_MENU
    Menu *menu;
#endif

    if (mi->d.input.nch == 0) {
	mi[3].flags = MenuButtonDisabled;	/* mi[i]: Save button! */
    } else {
	mi[3].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    SaveAction();
	    return;
	}
    }
    TypedFileName = 1;
#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
    menu->items[5].flags = MenuButtonDisabled;
#else
    mi->menu->items[5].flags = MenuButtonDisabled;
#endif
}

local void SaveAction(void)
{
    char filename[PATH_MAX];
    char *name;
    Menu *menu;
    size_t nameLength;
  
    menu = FindMenu("menu-save-game");
    name = menu->items[1].d.input.buffer;

    nameLength = strlen(name);
    if (TypedFileName) {
	nameLength -= 3;
    }

    strcpy(filename, SaveDir);
    strcat(filename, "/");
    strncat(filename, name, nameLength);
    if (strstr(filename, ".sav") == NULL)
	strcat(filename, ".sav");

    if (access(filename,F_OK)) {
        SaveGame(filename);
	SetMessage("Saved game to: %s", filename);
    } else {
	ProcessMenu("menu-save-confirm", 1);
    }

    EndMenu();
}

local void CreateSaveDir(void)
{
#ifdef USE_WIN32
    SaveDir="save";
    mkdir(SaveDir);
#else
    char path[PATH_MAX];

    strcpy(path,getenv("HOME"));
    strcat(path,"/");
    strcat(path,FREECRAFT_HOME_PATH);
    mkdir(path,0777);
    strcat(path,"/save");
    mkdir(path,0777);
    if (SaveDir)
	free(SaveDir);
    SaveDir = strdup(path);
#endif
}

global void GameMenuSave(void)
{
    char savegame_buffer[32];
    Menu *menu;

    menu = FindMenu("menu-save-game");
    strcpy(savegame_buffer, "~!_");
    menu->items[1].d.input.buffer = savegame_buffer;
    menu->items[1].d.input.nch = 0; /* strlen(savegame_buffer) - 3; */
    menu->items[1].d.input.maxch = 24;
    menu->items[4].flags = MenuButtonDisabled;	/* Save button! */
    ProcessMenu("menu-save-game", 1);
}

// FIXME: modify function
local void SaveLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local void SaveLBInit(Menuitem *mi)
{
    int i;
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-save-game");
#endif
    SaveLBExit(mi);

    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir,
	    NULL, (FileList **) & (mi->d.listbox.options));
    if (i != 0) {
#ifdef OLD_MENU
	if (i > 7) {
	    menu->items[3].flags = MI_ENABLED;
	} else {
	    menu->items[3].flags = MI_DISABLED;
	}
#else
	if (i > 7) {
	    mi->menu->items[3].flags = MI_ENABLED;
	} else {
	    mi->menu->items[3].flags = MI_DISABLED;
	}
#endif
    }
    mi->d.listbox.curopt = -1;
}

// FIXME: modify function
local unsigned char *SaveLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void SaveLBAction(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
	if (fl[i].type) {
	    strcpy(menu->items[1].d.input.buffer, fl[i].name);
	    menu->items[1].d.input.nch = strlen(fl[i].name);
	    menu->items[4].flags = MI_ENABLED;
	    menu->items[5].flags = MI_ENABLED;
	} else {
	    menu->items[4].flags = MenuButtonDisabled;
	    menu->items[5].flags = MenuButtonDisabled;
	}
    }
    TypedFileName = 0;
}

local void SaveVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi->d.listbox.curopt == -1)
	    mi->d.listbox.curopt = 0;

	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    SaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		SaveLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
    TypedFileName = 0;
}

// FIXME: modify function
local void SaveOk(void)
{
    FileList *fl;
    Menuitem *mi;
    Menu *menu;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

local int SaveRDFilter(char *pathbuf, FileList *fl)
{
    char *suf, *cp, *fsuffix, *np;

    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;

    suf = ".sav";

    do {
	fsuffix = cp++;
	cp = strcasestr(cp, suf);
    } while (cp != NULL);
    if (fsuffix >= np) {
	cp = fsuffix + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    fl->type='z';
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    fl->type='b';
	}
#endif
		if (strstr(pathbuf, ".sav")) {
		    if (fl->type == -1)
			fl->type = 'n';
		    fl->name = strdup(np);
		    fl->xdata = NULL;
		    return 1;
	}
    }
    return 0;
}

local void InitLoadGameMenu(Menuitem *mi)
{
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-load-game");
    menu->items[3].flags = MI_DISABLED;
#else
    mi->menu->items[3].flags = MI_DISABLED;
#endif
    CreateSaveDir();
}

// FIXME: modify function
local void LoadLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local void LoadLBInit(Menuitem *mi)
{
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-load-game");
#else
    menu = mi->menu;
#endif
    LoadLBExit(mi);

    i = mi->d.listbox.noptions = ReadDataDirectory(SaveDir, SaveRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i != 0) {
	if (i > 7) {
	    menu->items[2].flags = MenuButtonSelected;
	} else {
	    menu->items[2].flags = MenuButtonDisabled;
	}
    }
    mi->d.listbox.curopt = -1;
}

// FIXME: modify function
local unsigned char *LoadLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void LoadLBAction(Menuitem *mi, int i)
{
#ifdef OLD_MENU
    Menu *menu;
#endif
    FileList *fl;

#ifdef OLD_MENU
    menu = FindMenu("menu-load-game");
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (mi->d.listbox.noptions > 7) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
#ifdef OLD_MENU
	if (fl[i].type) {
	    menu->items[3].flags = MI_ENABLED;
	} else {
	    menu->items[3].flags = MI_DISABLED;
	}
#else
	if (fl[i].type) {
	    mi->menu->items[3].flags = MI_ENABLED;
	} else {
	    mi->menu->items[3].flags = MI_DISABLED;
	}
#endif
    }
}

local void LoadVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi->d.listbox.curopt == -1)
	    mi->d.listbox.curopt = 0;

	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    LoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		LoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

// FIXME: modify function
local void LoadOk(void)
{
    FileList *fl;
    Menuitem *mi;
    Menu *menu;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

#if 0
local void SaveMenu(void)
{
    EndMenu();
    ProcessMenu("menu-save-confirm", 1);
}
#endif

local void ConfirmSaveInit(Menuitem * mi __attribute__ ((unused)))
{
    static char name[PATH_MAX];		// FIXME: much memory wasted
    int fileLength;
    Menu *menu;

    menu = FindMenu("menu-save-game");
    fileLength = strlen(menu->items[1].d.input.buffer);
    if (TypedFileName) {
	fileLength -= 3;
    }

    strcpy(name, "the file: ");
    strncat(name, menu->items[1].d.input.buffer, fileLength);
    if (strstr(name, ".sav") == NULL) {
	strcat(name, ".sav");
    }
    menu = FindMenu("menu-save-confirm");
    menu->items[2].d.text.text = name;
}

local void ConfirmSaveFile(void)
{
    char name[PATH_MAX];
    int fileLength;
    Menu *menu;

    menu = FindMenu("menu-save-game");
    fileLength = strlen(menu->items[1].d.input.buffer);
    if (TypedFileName) {
	fileLength -= 3;
    }

    strcpy(name, SaveDir);
    strcat(name, "/");
    strncat(name, menu->items[1].d.input.buffer, fileLength);
    if (strstr(name, ".sav") == NULL) {
	strcat(name, ".sav");
    }
    SaveGame(name);
    SetMessage("Saved game to: %s", name);
    EndMenu();
}

local void FcDeleteMenu(void)
{
    EndMenu();
    ProcessMenu("menu-delete-confirm", 1);
}

local void FcDeleteInit(Menuitem *mi __attribute__((unused)))
{
    static char name[PATH_MAX];		// FIXME: much memory wasted
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-save-game");
#else
    menu = mi->menu;
#endif
    strcpy(name, "the file: ");
    strcat(name, menu->items[1].d.input.buffer);
    menu = FindMenu("menu-delete-confirm");
    menu->items[2].d.text.text = name;
}
  
local void FcDeleteFile(void)
{
    Menu *menu;
    char name[PATH_MAX];

    menu = FindMenu("menu-save-game");
    strcpy(name, SaveDir);
    strcat(name, "/");
    strcat(name, menu->items[1].d.input.buffer);
    unlink(name);
    EndMenu();
    *menu->items[1].d.input.buffer = '\0';
    menu->items[1].d.input.nch = 0;
    ProcessMenu("menu-save-game", 1);
}

local void LoadAction(void)
{
    Menu *menu;
    char filename[256];
    FileList *fl;
    char *name;
    size_t nameLength;

    menu = FindMenu("menu-load-game");
    fl = menu->items[1].d.listbox.options;

    name = fl[menu->items[1].d.listbox.curopt].name;
    nameLength = strlen(name);

    strcpy(filename, SaveDir);
    strcat(filename, "/");
    strncat(filename, name, nameLength);

    LoadGame(filename);
    SetMessage("Loaded game: %s", filename);
    GameLoaded=1;
    EndMenu();
}

global void GameMenuLoad(void)
{
    Menu *menu;

    menu = FindMenu("menu-load-game");
    menu->items[3].flags = MI_DISABLED;		// Load button!
    GameLoaded=0;
    ProcessMenu("menu-load-game", 1);
    if( GameLoaded ) {
	GameMenuReturn();
    }
}

local void InitGameMenu(Menuitem *mi __attribute__((unused)))
{
    // FIXME: populate...
}

global void SoundOptions(void)
{
    ProcessMenu("menu-sound-options", 1);
}

local void InitSoundOptions(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
    Menu *menu;

    menu = FindMenu("menu-sound-options");

    // master volume slider
    if (SoundFildes == -1) {
	menu->items[2].flags = -1;
    } else {
	menu->items[2].flags = 0;
	menu->items[2].d.hslider.percent = (GlobalVolume * 100) / 255;
    }

    // master power
    if (SoundFildes == -1) {
	menu->items[5].d.gem.state = MI_GSTATE_UNCHECKED;
    } else {
	menu->items[5].d.gem.state = MI_GSTATE_CHECKED;
    }

    // music volume slider
    if (PlayingMusic != 1 || SoundFildes == -1) {
	menu->items[8].flags = -1;
    } else {
	menu->items[8].flags = 0;
	menu->items[8].d.hslider.percent = (MusicVolume * 100) / 255;
    }

    // music power
    if (SoundFildes == -1) {
	menu->items[11].flags = -1;
    } else {
	menu->items[11].flags = 0;
    }
    if (strcmp(":off", CDMode) && strcmp(":stopped", CDMode)) {
	menu->items[11].flags = -1;
    }
    if (PlayingMusic != 1 || SoundFildes == -1) {
	menu->items[11].d.gem.state = MI_GSTATE_UNCHECKED;
    } else {
	menu->items[11].d.gem.state = MI_GSTATE_CHECKED;
    }

#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
	menu->items[14].flags = -1;		// cd volume slider
	menu->items[17].d.gem.state = MI_GSTATE_UNCHECKED; // cd power
	menu->items[19].flags = -1;		// all tracks button
	menu->items[21].flags = -1;		// random tracks button
    } else {
#ifdef USE_LIBCDA
	int i = 0;

	cd_get_volume(&i, &i);
	menu->items[14].d.hslider.percent = (i * 100) / 255;
#endif
	if (!strcmp(":all", CDMode)) {
	    menu->items[19].d.gem.state = MI_GSTATE_CHECKED;
	}
	if (!strcmp(":random", CDMode)) {
	    menu->items[21].d.gem.state = MI_GSTATE_CHECKED;
	}
	menu->items[11].flags = -1;
    }
#else // without cd
    menu->items[14].flags = -1;			// cd volume slider
    menu->items[17].flags = -1;			// cd power
    menu->items[17].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[19].flags = -1;			// all tracks button
    menu->items[21].flags = -1;			// random tracks button
#endif // cd
#endif // with sound
}

local void GlobalOptions(void)
{
    ProcessMenu("menu-global-options", 1);
}

local void InitGlobalOptions(Menuitem *mi __attribute__((unused)))
{
    Menu *menu;

    menu = FindMenu("menu-global-options");
    
    menu->items[2].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[4].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[6].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[8].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[10].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[15].d.gem.state = MI_GSTATE_UNCHECKED;
    menu->items[17].d.gem.state = MI_GSTATE_UNCHECKED;

    if (VideoWidth == 640) {
	menu->items[2].d.gem.state = MI_GSTATE_CHECKED;
    } else if (VideoWidth == 800) {
    	menu->items[4].d.gem.state = MI_GSTATE_CHECKED;
    } else if (VideoWidth == 1024) {
    	menu->items[6].d.gem.state = MI_GSTATE_CHECKED;
    } else if (VideoWidth == 1280) {
    	menu->items[8].d.gem.state = MI_GSTATE_CHECKED;
    } else if (VideoWidth == 1600) {
    	menu->items[10].d.gem.state = MI_GSTATE_CHECKED;
    }

    if (VideoFullScreen) {
	menu->items[12].d.gem.state = MI_GSTATE_CHECKED;
    }

    if (OriginalFogOfWar) {
	menu->items[15].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[17].d.gem.state = MI_GSTATE_CHECKED;
    }
}

local void SetRes(Menuitem *mi)
{
    Menu *menu;
    int res;

    menu = FindMenu("menu-global-options");

    res=VideoWidth;
    if (mi[+1].d.text.text == menu->items[3].d.text.text) {
	res=640;
    } else if (mi[+1].d.text.text == menu->items[5].d.text.text) {
	res=800;
    } else if (mi[+1].d.text.text == menu->items[7].d.text.text) {
	res=1024;
    } else if (mi[+1].d.text.text == menu->items[9].d.text.text) {
	res=1280;
    } else if (mi[+1].d.text.text == menu->items[11].d.text.text) {
	res=1600;
    }

    if (VideoWidth != res) {
	VideoWidth=res;
	VideoHeight=res * 3 / 4;
	InitVideo();
	DestroyCursorBackground();
	SetClipping(0,0,VideoWidth-1,VideoHeight-1);
	LoadCcl();
    }
    InitGlobalOptions(NULL);
}

local void SetFullscreen(Menuitem *mi __attribute__((unused)))
{
    ToggleFullScreen();
    InitGlobalOptions(NULL);
}

local void SetShadowFogAlpha(Menuitem *mi __attribute__((unused)))
{
    OriginalFogOfWar=1;
    InitGlobalOptions(NULL);
}

local void SetShadowFogGray(Menuitem *mi __attribute__((unused)))
{
    OriginalFogOfWar=0;
    InitGlobalOptions(NULL);
}

local void SetMasterPower(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
    if (SoundFildes != -1) {
#ifdef USE_SDLA
	SDL_CloseAudio();
#else
        close(SoundFildes);
#endif
	SoundFildes=-1;
	SoundOff=1;
    } else {
	SoundOff=0;
	if( InitSound() ) {
	    SoundOff=1;
	    SoundFildes=-1;
	}
	MapUnitSounds();
	InitSoundServer();
	InitSoundClient();
    }
#endif
    InitSoundOptions(NULL);
}

local void SetMusicPower(Menuitem *mi __attribute__((unused)))
{
#ifdef WITH_SOUND
    SCM cb;

    if (PlayingMusic == 1) {
	StopMusic();
    } else {
	if (CallbackMusic) {
	    cb = gh_symbol2scm("music-stopped");
	    if (!gh_null_p(symbol_boundp(cb, NIL))) {
		SCM value;

		value = symbol_value(cb, NIL);
		if (!gh_null_p(value)) {
		    gh_apply(value, NIL);
		}
	    }
	}
    }
#endif // with sound
    InitSoundOptions(NULL);
}

local void SetCdPower(Menuitem *mi __attribute__((unused)))
{
#ifdef USE_SDLCD
    // Start Playing CD
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
#ifdef USE_WIN32
	SDL_CDResume(CDRom);
#endif
	PlayMusic(":random");
    } else {
    // Stop Playing CD
        SDL_CDPause(CDRom);
	CDMode = ":stopped";
    }
#elif defined(USE_LIBCDA)
    // Start Playing CD
    if (!strcmp(":off", CDMode) || !strcmp(":stopped", CDMode)) {
	PlayMusic(":random");
    } else {
    // Stop Playing CD
        cd_pause();
	CDMode = ":stopped";
    }
#endif
    InitSoundOptions(NULL);
}

/**
**	Toggle the map of war handling.
**	Used in the preference menu.
**
**	@param mi	Menu item (not used).
*/
local void SetFogOfWar(Menuitem *mi __attribute__((unused)))
{
    if (!TheMap.NoFogOfWar) {
        TheMap.NoFogOfWar = 1;
        UpdateFogOfWarChange();
        MapUpdateVisible();
    } else {
        TheMap.NoFogOfWar = 0;
        UpdateFogOfWarChange();
        MapUpdateVisible();
    }
    MustRedraw &= ~RedrawMinimap;
}

/**
**	Toggle showing the command chars on icons.
**	Used in the preference menu.
**
**	@param mi	Menu item (not used).
*/
local void SetCommandKey(Menuitem *mi __attribute__((unused)))
{
    ShowCommandKey ^= 1;
}

local void SetCdModeAll(Menuitem *mi __attribute__((unused)))
{
#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    CDMode = ":all";
#endif
    CurrentMenu=NULL;
    SoundOptions();

}

local void SetCdModeRandom(Menuitem *mi __attribute__((unused)))
{
#if defined(USE_LIBCDA) || defined(USE_SDLCD)
    CDMode = ":random";
#endif
    CurrentMenu=NULL;
    SoundOptions();

}

global void SpeedSettings(void)
{
    Menu *menu;
    int i = 2;

    menu = FindMenu("menu-settings-speed");
    menu->items[i].d.hslider.percent = ((VideoSyncSpeed - MIN_GAME_SPEED) * 100) / (MAX_GAME_SPEED - MIN_GAME_SPEED);
    if (menu->items[i].d.hslider.percent < 0)
	menu->items[i].d.hslider.percent = 0;
    if (menu->items[i].d.hslider.percent > 100)
	menu->items[i].d.hslider.percent = 100;
    menu->items[i + 4].d.hslider.percent = 100 - (SpeedMouseScroll - 1) * 100 / 10;
    if (TheUI.MouseScroll == 0)
	menu->items[i + 4].d.hslider.percent = 0;
    menu->items[i + 8].d.hslider.percent = 100 - (SpeedKeyScroll - 1) * 100 / 10;
    if (TheUI.KeyScroll == 0)
	menu->items[i + 8].d.hslider.percent = 0;
    ProcessMenu("menu-settings-speed", 1);
}

/**
**	Enter the preferences menu.
**	Setup if the options are enabled / disabled.
**
**	@todo	FIXME: The init should be done by the init callback.
*/
global void Preferences(void)
{
    Menu *menu;

    menu = FindMenu("menu-preferences");
    if (!TheMap.NoFogOfWar) {
	menu->items[1].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    if (NetworkFildes == -1) {		// Not available in net games
	menu->items[1].flags = MI_ENABLED;
    } else {
	menu->items[1].flags = MI_DISABLED;
    }

    if (ShowCommandKey) {
	menu->items[3].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[3].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    ProcessMenu("menu-preferences", 1);
}

/**
**	Show the game options.
*/
local void GameOptions(void)
{
    ProcessMenu("menu-game-options", 1);
}

/**
**	Show the global options.
*/
local void GameGlobalOptionsMenu(void)
{
    // FIXME: write me
}

/**
**	Show the game credits.
*/
local void GameShowCredits(void)
{
    ShowCredits(&GameCredits);
}

/**
**	Show the End Scenario menu
*/
local void GameMenuEndScenario(void)
{
    ProcessMenu("menu-end-scenario", 1);
    if (!GameRunning) {
	EndMenu();
	InterfaceState=IfaceStateNormal;
    }
}

/**
**	Restart the scenario
*/
local void EndScenarioRestart(void)
{
    RestartScenario=1;
    GameRunning=0;
    EndMenu();
}

/**
**	End the game in defeat
*/
local void EndScenarioSurrender(void)
{
    GameResult=GameDefeat;
    GameRunning=0;
    EndMenu();
}

/**
**	End the game and return to the menu
*/
local void EndScenarioQuitMenu(void)
{
    QuitToMenu=1;
    GameRunning=0;
    EndMenu();
}

/**
**	End the running game from menu.
*/
local void GameMenuEnd(void)
{
    InterfaceState=IfaceStateNormal;
    GameRunning=0;
    CursorOn=CursorOnUnknown;
    CurrentMenu=NULL;
}

local void KeystrokeHelpMenu(void)
{
    Menu *menu;

    menu = FindMenu("menu-keystroke-help");
    menu->items[1].d.vslider.percent = 0;
    ProcessMenu("menu-keystroke-help", 1);
}

local void HelpMenu(void)
{
    ProcessMenu("menu-help", 1);
}

local void ShowTipsMenu(void)
{
    Menu *menu;

    menu = FindMenu("menu-tips");
    if (ShowTips) {
	menu->items[1].d.gem.state = MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state = MI_GSTATE_UNCHECKED;
    }

    ProcessMenu("menu-tips", 1);
}

/**
**	Free the tips from the menu
*/
local void FreeTips()
{
    int i;
    Menu *menu;

    menu = FindMenu("menu-tips");
    for( i=5; i<13; i++ ) {
	if( menu->items[i].d.text.text ) {
	    free(menu->items[i].d.text.text);
	    menu->items[i].d.text.text=NULL;
	}
    }
}

/**
**	Initialize the tips menu
*/
local void InitTips(Menuitem *mi __attribute__((unused)))
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;
    Menu *menu;

    menu = FindMenu("menu-tips");

    if( ShowTips ) {
	menu->items[1].d.gem.state=MI_GSTATE_CHECKED;
    } else {
	menu->items[1].d.gem.state=MI_GSTATE_UNCHECKED;
    }

    FreeTips();

    w=menu->xsize-2*menu->items[5].xofs;
    font=menu->items[5].font;
    i=0;
    line=5;

    p=Tips[CurrentTip];
    if( !p ) {
	return;
    }

    l=0;
    s=str=strdup(p);

    while( line<13 ) {
	char* s1;
	char* space;

	space=NULL;
	for( ;; ) {
	    if( VideoTextLength(font,s)<w ) {
		break;
	    }
	    s1=strrchr(s,' ');
	    if( !s1 ) {
		fprintf(stderr, "line too long: \"%s\"\n", s);
		break;
	    }
	    if( space ) {
		*space=' ';
	    }
	    space=s1;
	    *space='\0';
	}
	menu->items[line++].d.text.text=strdup(s);
	l+=strlen(s);
	if( !p[l] ) {
	    break;
	}
	++l;
	s=str+l;
    }

    free(str);
}

/**
**	Show tips at startup gem callback
*/
#ifdef OLD_MENU
local void SetTips(Menuitem *mi __attribute__((unused)))
#else
local void SetTips(Menuitem *mi)
#endif
{
#ifdef OLD_MENU
    Menu *menu;

    menu = FindMenu("menu-tips");
    if (menu->items[1].d.gem.state == MI_GSTATE_CHECKED)
#else
    if (mi->menu->items[1].d.gem.state == MI_GSTATE_CHECKED)
#endif
	 {
	ShowTips = 1;
    } else {
	ShowTips = 0;
    }
}

local void NextTip(void)
{
    CurrentTip++;
    if (Tips[CurrentTip] == NULL)
	CurrentTip = 0;
}

/**
**	Cycle through the tips
*/
local void ShowNextTip(void)
{
    NextTip();
    InitTips(NULL);
}

/**
**	Exit the tips menu
*/
local void TipsMenuEnd(void)
{
    NextTip();
    FreeTips();

    EndMenu();
}


/**
**	Exit the game from menu.
*/
local void GameMenuExit(void)
{
    Exit(0);
}

/**
**
*/
local void SetMenuObjectives(void)
{
    int i;
    int line;
    char* p;
    char* s;
    char* str;
    int w;
    int font;
    int l;
    Menu *menu;

    menu = FindMenu("menu-objectives");
    w=menu->xsize-2*menu->items[1].xofs;
    font=menu->items[1].font;
    i=0;
    line=1;

    for( p=GameIntro.Objectives[i]; p; p=GameIntro.Objectives[++i] ) {
	l=0;
	s=str=strdup(p);

	for( ;; ) {
	    char* s1;
	    char* space;

	    space=NULL;
	    for( ;; ) {
		if( VideoTextLength(font,s)<w ) {
		    break;
		}
		s1=strrchr(s,' ');
		if( !s1 ) {
		    fprintf(stderr, "line too long: \"%s\"\n", s);
		    break;
		}
		if( space )
		    *space=' ';
		space=s1;
		*space='\0';
	    }
	    menu->items[line++].d.text.text=strdup(s);
	    l+=strlen(s);
	    if( !p[l] ) {
		break;
	    }
	    ++l;
	    s=str+l;

	    if( line==menu->nitems-1 ) {
		break;
	    }
	}
	free(str);
    }
}

/**
**
*/
local void FreeMenuObjectives(void)
{
    int i;
    Menu *menu;

    menu = FindMenu("menu-objectives");
    for( i=1;i<menu->nitems-1;i++ ) {
	if( menu->items[i].d.text.text ) {
	    free(menu->items[i].d.text.text);
	    menu->items[i].d.text.text=NULL;
	}
    }
}

/**
**
*/
local void GameMenuObjectives(void)
{
    SetMenuObjectives();
    ProcessMenu("menu-objectives", 1);
    FreeMenuObjectives();
}

/**
**	Get pud info from select path+name
*/
local void GetInfoFromSelectPath(void)
{
    int i;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    if (ScenSelectPath[0]) {
	i = strlen(ScenSelectPath);
	strcat(ScenSelectPath, "/");
    } else {
	i = 0;
    }
    strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
    if (strcasestr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectPath);
	strcpy(ScenSelectFullPath, ScenSelectPath);
    } else {
	// FIXME: GetCmInfo();
    }
    ScenSelectPath[i] = '\0';		// Remove appended part
}

/**
**	Enter select scenario menu.
*/
local void ScenSelectMenu(void)
{
    ProcessMenu("menu-select-scenario", 1);

    GetInfoFromSelectPath();
}

/**
**	Enter multiplayer select scenario menu.
*/
local void MultiScenSelectMenu(void)
{
    ScenSelectMenu();
    MultiGamePlayerSelectorsUpdate(1);
}

/**
**	Enter single player menu.
*/
local void SinglePlayerGameMenu(void)
{
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu("menu-custom-game", 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

/**
**	Show the campaign select menu.
**
**	Look which campaigns are available and how they are called.
*/
local void CampaignGameMenu(void)
{
    int i;
    Menu *menu;

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

    menu = FindMenu("menu-campaign-select");
    DebugLevel0Fn("%d campaigns available\n" _C_ NumCampaigns);
    IfDebug(
	for( i=0; i<NumCampaigns; ++i ) {
	    DebugLevel0Fn("Campaign %d: %16.16s: %s\n" _C_ i _C_
		Campaigns[i].Ident _C_
		Campaigns[i].Name);
	}
    );

    //
    //	Setup campaign name.
    //
    for( i=0; i<NumCampaigns && i<4; ++i ) {
	char* s;

	menu->items[i].d.button.text=Campaigns[i].Name;
	menu->items[i].flags&=~MenuButtonDisabled;

	if( (s=strchr(Campaigns[i].Name,'!')) ) {
	    menu->items[i].d.button.hotkey=tolower(s[1]);
	}
    }
    for( ; i<4; ++i ) {
	menu->items[i].d.button.text="Not available";
	menu->items[i].flags|=MenuButtonDisabled;
    }

    GuiGameStarted = 0;
    ProcessMenu("menu-campaign-select", 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }

    for( i=0; i<4; ++i ) {
	menu->items[i].d.button.text=NULL;
    }
}

/**
**	Start campaign from menu.
**
**	@param number	Number of the compaign.
*/
local void StartCampaignFromMenu(int number)
{
    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

#if 0
    // JOHNS: this is currently not needed:

    // Any Campaign info should be displayed through a DrawFunc() Item
    // int the CAMPAIN_CONT menu processed below...
    ProcessMenu("menu-campaign-continue", 1);
    // Set GuiGameStarted = 1 to acctually run a game here...
    // See CustomGameStart() for info...
#endif

    PlayCampaign(Campaigns[number].Ident);
    GuiGameStarted = 1;

    VideoLockScreen();
    MenusSetBackground();
    VideoClearScreen();
    VideoUnlockScreen();
    Invalidate();

    // FIXME: johns otherwise crash in UpdateDisplay -> DrawMinimapCursor
    EndMenu();
}

/**
**	Call back for 1st entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu1(void)
{
    StartCampaignFromMenu(0);
}

/**
**	Call back for 2nd entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu2(void)
{
    StartCampaignFromMenu(1);
}

/**
**	Call back for 3rd entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu3(void)
{
    StartCampaignFromMenu(2);
}

/**
**	Call back for 4th entry of campaign menu.
**
**	@note FIXME: Isn't it possible to have an argument in the menu?
*/
local void CampaignMenu4(void)
{
    StartCampaignFromMenu(3);
}

/**
**	Call back for select entry of campaign menu.
*/
local void SelectCampaignMenu(void)
{
    // FIXME: not written
}

/**
**	Cancel button of player name menu pressed.
*/
local void EnterNameCancel(void)
{
    Menu *menu;
    menu = FindMenu("menu-enter-name");
    menu->items[1].d.input.nch = 0;
    EndMenu();
}

/**
**	Input field action of player name menu.
*/
local void EnterNameAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[1].flags = MenuButtonDisabled;
    } else {
	mi[1].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    EndMenu();
	}
    }
}

/**
**	Cancel button of enter server ip/name menu pressed.
*/
local void EnterServerIPCancel(void)
{
    Menu *menu;
    menu = FindMenu("menu-enter-server");
    menu->items[1].d.input.nch = 0;
    EndMenu();
}

/**
**	Input field action of server ip/name.
*/
local void EnterServerIPAction(Menuitem *mi, int key)
{
    if (mi->d.input.nch == 0) {
	mi[1].flags = MenuButtonDisabled;
    } else {
	mi[1].flags &= ~MenuButtonDisabled;
	if (key == 10 || key == 13) {
	    EndMenu();
	}
    }
}

/**
**	Start processing join a network game menu.
*/
local void JoinNetGameMenu(void)
{
    char server_host_buffer[32];
    Menu *menu;

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

    //
    //  Prepare enter ip/hostname menu
    //
    if (NetworkArg) {
	strcpy(server_host_buffer, NetworkArg);
    } else {
	server_host_buffer[0] = '\0';
    }
    menu = FindMenu("menu-enter-server");
    strcat(server_host_buffer, "~!_");
    menu->items[1].d.input.buffer = server_host_buffer;
    menu->items[1].d.input.nch = strlen(server_host_buffer) - 3;
    menu->items[1].d.input.maxch = 24;
    if (menu->items[1].d.input.nch) {
	menu->items[2].flags &= ~MenuButtonDisabled;
    } else {
	menu->items[2].flags |= MenuButtonDisabled;
    }

    ProcessMenu("menu-enter-server", 1);

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();

    if (menu->items[1].d.input.nch == 0) {
	return;
    }
    // Now finally here is the address
    server_host_buffer[menu->items[1].d.input.nch] = 0;
    if (NetworkSetupServerAddress(server_host_buffer)) {
	menu = FindMenu("menu-net-error");
	menu->items[1].d.text.text = "Unable to lookup host.";
	ProcessMenu("menu-net-error", 1);
	VideoLockScreen();
	MenusSetBackground();
	VideoUnlockScreen();
	return;
    }
    NetworkInitClientConnect();
    if (NetworkArg) {
	free(NetworkArg);
    }
    NetworkArg = strdup(server_host_buffer);

    // Here we really go...
    ProcessMenu("menu-net-connecting", 1);

    if (GuiGameStarted) {
	VideoLockScreen();
	MenusSetBackground();
	VideoUnlockScreen();
	Invalidate();
	EndMenu();
    }
}

/**
**	Cancel button of network connect menu pressed.
*/
local void NetConnectingCancel(void)
{
    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    NetworkExitClientConnect();
    // Trigger TerminateNetConnect() to call us again and end the menu
    NetLocalState = ccs_usercanceled;
    EndMenu();
}

/**
**	Call back from menu loop, if network state has changed.
*/
local void TerminateNetConnect(void)
{
    Menu *menu;

    menu = FindMenu("menu-net-error");
    switch (NetLocalState) {
	case ccs_unreachable:
	    menu->items[1].d.text.text = "Cannot reach server.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_nofreeslots:
	    menu->items[1].d.text.text = "Server is full.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_serverquits:
	    menu->items[1].d.text.text = "Server gone.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatibleengine:
	    menu->items[1].d.text.text = "Incompatible engine version.";
	    ProcessMenu("menu-net-error", 1);

	    NetConnectingCancel();
	    return;
	case ccs_incompatiblenetwork:
	    menu->items[1].d.text.text = "Incompatible network version.";
	    ProcessMenu("menu-net-error", 1);

	case ccs_usercanceled:
	    NetConnectingCancel();
	    return;

	case ccs_started:
	    NetworkGamePrepareGameSettings();
	    CustomGameStart();
	    return;

	default:
	    break;
    }

    DebugLevel1Fn("NetLocalState %d\n" _C_ NetLocalState);
    NetConnectRunning = 2;
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu("menu-net-multi-client", 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    } else {
	NetConnectingCancel();
    }
}

/**
**	Start processing network game setup menu (server).
*/
local void CreateNetGameMenu(void)
{
    DestroyCursorBackground();
    GuiGameStarted = 0;
    ProcessMenu("menu-multi-setup", 1);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

/**
**	Multiplayer game start game button pressed.
*/
local void MultiGameStart(void)
{
    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

    NetworkServerStartGame();
    NetworkGamePrepareGameSettings();

    CustomGameStart();
}

/**
**	Enter multiplayer game menu.
*/
local void MultiPlayerGameMenu(void)
{
    char NameBuf[32];
    Menu *menu;

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

    menu = FindMenu("menu-enter-name");
    menu->items[1].d.input.buffer = NameBuf;
    strcpy(NameBuf, NetworkName);
    strcat(NameBuf, "~!_");
    menu->items[1].d.input.nch = strlen(NameBuf) - 3;
    menu->items[1].d.input.maxch = 15;
    menu->items[2].flags &= ~MenuButtonDisabled;

    ProcessMenu("menu-enter-name", 1);

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();

    if (menu->items[1].d.input.nch == 0) {
	return;
    }

    NameBuf[menu->items[1].d.input.nch] = 0;	// Now finally here is the name
    memset(NetworkName, 0, 16);
    strcpy(NetworkName, NameBuf);

    GuiGameStarted = 0;
    // Here we really go...
    ProcessMenu("menu-create-join-menu", 1);

    DebugLevel0Fn("GuiGameStarted: %d\n" _C_ GuiGameStarted);
    if (GuiGameStarted) {
	GameMenuReturn();
    }
}

local void FreeMapInfos(FileList *fl, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	if (fl[i].type && fl[i].xdata) {
	    FreeMapInfo(fl[i].xdata);
	    fl[i].xdata = NULL;
	}
    }
}

/**
**	Initialize the scenario selector menu.
*/
#ifdef OLD_MENU
local void ScenSelectInit(Menuitem *mi __attribute__ ((unused)))
#else
local void ScenSelectInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    DebugCheck(!*ScenSelectPath);
    menu->items[9].flags =
	*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
    menu->items[9].d.button.text = ScenSelectDisplayPath;
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

local void ScenSelectLBAction(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    menu->items[3].d.button.text = "OK";
	} else {
	    menu->items[3].d.button.text = "Open";
	}
	if (mi->d.listbox.noptions > 5) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
    }
}

local void ScenSelectLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	FreeMapInfos(fl, mi->d.listbox.noptions);
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local int ScenSelectRDFilter(char *pathbuf, FileList *fl)
{
    MapInfo *info;
    char *suf, *cp, *lcp, *np;
    int p, sz;
    static int szl[] = { -1, 32, 64, 96, 128, 256, 512, 1024 };
    Menu *menu;
#ifdef USE_ZZIPLIB
    ZZIP_FILE *zzf;
#endif

    menu = FindMenu("menu-select-scenario");

#if 0
    if (menu->items[6].d.pulldown.curopt == 0) {
	suf = NULL;
	p = -1;
    } else
#endif
    if (menu->items[6].d.pulldown.curopt == 0) {
	suf = ".cm";
	p = 0;
    } else {
	suf = ".pud";
	p = 1;
    }
    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;
#ifdef USE_ZZIPLIB
    if ((zzf = zzip_open(pathbuf, O_RDONLY|O_BINARY))) {
	sz = zzip_file_real(zzf);
	zzip_close(zzf);
	if (!sz) {
	    goto usezzf;
	}
    }
#endif
    do {
	lcp = cp++;
	cp = strcasestr(cp, suf);
#if 0
	if( suf ) {
	} else if( !suf && (cp = strcasestr(cp, ".cm")) ) {
	    suf = ".cm";
	    p = 0;
	} else if( !suf && (cp = strcasestr(cp, ".pud")) ) {
	    suf = ".pud";
	    p = 1;
	} else {
	    cp=NULL;
	}
#endif
    } while (cp != NULL);
    if (lcp >= np) {
	cp = lcp + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    *cp = 0;
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    *cp = 0;
	}
#endif
	if (*cp == 0) {
#ifdef USE_ZZIPLIB
usezzf:
#endif
#if 0
	    if( p==-1 ) {
		printf("What now ?\n");
		if (strcasestr(pathbuf, ".pud")) {
		    p=1;
		} else {
		    p=0;
		}
	    }
#endif
	    if (p) {
		if (strcasestr(pathbuf, ".pud")) {
		    info = GetPudInfo(pathbuf);
		    if (info) {
			DebugLevel3Fn("GetPudInfo(%s) : %p\n" _C_ pathbuf _C_ info);
			sz = szl[menu->items[8].d.pulldown.curopt];
			if (sz < 0 || (info->MapWidth == sz && info->MapHeight == sz)) {
			    fl->type = 1;
			    fl->name = strdup(np);
			    fl->xdata = info;
			    return 1;
			} else {
			    FreeMapInfo(info);
			}
		    }
		}
	    } else {
		if (strstr(pathbuf, ".cm")) {
		    // info = GetCmInfo(pathbuf);
		    info = NULL;
		    DebugLevel3Fn("GetCmInfo(%s) : %p\n" _C_ pathbuf _C_ info);
		    fl->type = 1;
		    fl->name = strdup(np);
		    fl->xdata = info;
		    return 1;
		}
	    }
	}
    }
    return 0;
}

local void ScenSelectLBInit(Menuitem *mi)
{
    int i;
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-select-scenario");
#else
    menu = mi->menu;
#endif
    ScenSelectLBExit(mi);
    if (menu->items[6].d.pulldown.curopt == 0) {
	menu->items[8].flags |= MenuButtonDisabled;
    } else {
	menu->items[8].flags &= ~MenuButtonDisabled;
    }
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, ScenSelectRDFilter,
						     (FileList **)&(mi->d.listbox.options));
    if (i == 0) {
	menu->items[3].d.button.text = "OK";
	menu->items[3].flags |= MenuButtonDisabled;
    } else {
	ScenSelectLBAction(mi, 0);
	menu->items[3].flags &= ~MenuButtonDisabled;
	if (i > 5) {
	    mi[1].flags &= ~MenuButtonDisabled;
	}
    }
}

local unsigned char *ScenSelectLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
#ifdef OLD_MENU
		    menu = FindMenu("menu-select-scenario");
#else
		    menu = mi->menu;
#endif
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+254,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+254+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void ScenSelectTPMSAction(Menuitem *mi, int i __attribute__((unused)))
{
#ifdef OLD_MENU
    Menu *menu;
    menu = FindMenu("menu-select-scenario");
    mi = menu->items + 1;
#else
    mi = mi->menu->items + 1;
#endif
    ScenSelectLBInit(mi);
    mi->d.listbox.cursel = -1;
    mi->d.listbox.startline = 0;
    mi->d.listbox.curopt = 0;
    mi[1].d.vslider.percent = 0;
    mi[1].d.hslider.percent = 0;
    MustRedraw |= RedrawMenu;
}

local void ScenSelectVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		ScenSelectLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void KeystrokeHelpVSAction(Menuitem *mi, int i)
{
    int j, nitems = nKeyStrokeHelps;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    j = ((mi->d.vslider.percent + 1) * (nitems - 11)) / 100;
	    if (mi->d.vslider.cflags&MI_CFLAGS_DOWN && j < nitems - 11) {
		    j++;
		    MustRedraw |= RedrawMenu;
	    } else if (mi->d.vslider.cflags&MI_CFLAGS_UP && j > 0) {
		    j--;
		    MustRedraw |= RedrawMenu;
	    }
	    if (i == 2) {
		mi->d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    mi->d.vslider.percent = j * 100 / (nitems - 11);
	    break;
	case 1:		// mouse - move
	    if ((mi->d.vslider.cflags&MI_CFLAGS_KNOB) && (mi->flags&MenuButtonClicked)) {
		j = ((mi->d.vslider.curper + 1) * (nitems - 11)) / 100;
		mi->d.vslider.percent = mi->d.vslider.curper;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

/**
**	Draw the Keystroke Help texts
*/
local void KeystrokeHelpDrawFunc(Menuitem *mi)
{
    int i, j, nitems = nitems = nKeyStrokeHelps;
#ifdef OLD_MENU
    Menu *menu = FindMenu("menu-keystroke-help");
#else
    Menu *menu = mi->menu;
#endif

    j = ((mi[-2].d.vslider.percent + 1) * (nitems - 11)) / 100;
    for (i = 0; i < 11; i++) {
	VideoDrawText(menu->x+mi->xofs,menu->y+mi->yofs+(i*20),
			    mi->font,KeyStrokeHelps[j*2]);
	VideoDrawText(menu->x+mi->xofs+80,menu->y+mi->yofs+(i*20),
			    mi->font,KeyStrokeHelps[j*2+1]);
	j++;
    }
}

local void GameSpeedHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing game speed by 10%s\n" _C_ "%");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing game speed by 10%s\n" _C_ "%");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
		VideoSyncSpeed = (mi[1].d.hslider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
		SetVideoSync();
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		VideoSyncSpeed = (mi[1].d.hslider.percent * (MAX_GAME_SPEED - MIN_GAME_SPEED)) / 100 + MIN_GAME_SPEED;
		SetVideoSync();
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MouseScrollHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing mouse speed\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing mouse speed\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    TheUI.MouseScroll = 1;
	    SpeedMouseScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
	    if (mi[1].d.hslider.percent == 0)
		TheUI.MouseScroll = 0;
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		TheUI.MouseScroll = 1;
		SpeedMouseScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
		if (mi[1].d.hslider.percent == 0)
		    TheUI.MouseScroll = 0;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void KeyboardScrollHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing keyboard speed\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing keyboard speed\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    TheUI.KeyScroll = 1;
	    SpeedKeyScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
	    if (mi[1].d.hslider.percent == 0)
		TheUI.KeyScroll = 0;
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		TheUI.KeyScroll = 1;
		SpeedKeyScroll = 10 - (mi[1].d.hslider.percent * 9) / 100;
		if (mi[1].d.hslider.percent == 0)
		    TheUI.KeyScroll = 0;
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MasterVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing master volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing master volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    SetGlobalVolume((mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		SetGlobalVolume((mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void MusicVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing music volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing music volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    SetMusicVolume((mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		SetMusicVolume((mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

#ifdef USE_LIBCDA
local void CdVolumeHSAction(Menuitem *mi, int i)
{
    mi--;

    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_RIGHT) {
		DebugLevel0Fn("Increasing cd volume\n");
		mi[1].d.hslider.percent += 10;
		if (mi[1].d.hslider.percent > 100)
		    mi[1].d.hslider.percent = 100;
	    } else if (mi[1].d.hslider.cflags&MI_CFLAGS_LEFT) {
		DebugLevel0Fn("Decreasing cd volume\n");
		mi[1].d.hslider.percent -= 10;
		if (mi[1].d.hslider.percent < 0)
		    mi[1].d.hslider.percent = 0;
	    }
	    if (i == 2) {
		mi[1].d.hslider.cflags &= ~(MI_CFLAGS_RIGHT|MI_CFLAGS_LEFT);
	    }
	    cd_set_volume((mi[1].d.hslider.percent * 255) / 100,(mi[1].d.hslider.percent * 255) / 100);
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.hslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.hslider.curper > mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		} else if (mi[1].d.hslider.curper < mi[1].d.hslider.percent) {
		    mi[1].d.hslider.percent = mi[1].d.hslider.curper;
		}
		mi[1].d.hslider.percent = (mi[1].d.hslider.curper + 6) / 10 * 10;
		cd_set_volume((mi[1].d.hslider.percent * 255) / 100,(mi[1].d.hslider.percent * 255) / 100);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}
#else
local void CdVolumeHSAction(Menuitem *mi __attribute__((unused)),
	int i __attribute__((unused)))
{
}
#endif

local void ScenSelectFolder(void)
{
    char *cp;
    Menu *menu;
    Menuitem *mi;

    menu = FindMenu("menu-select-scenario");
    mi = menu->items + 1;
    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    menu->items[9].flags |= MenuButtonDisabled;
	    menu->items[9].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	}
    }
}

local void ScenSelectOk(void)
{
    FileList *fl;
    Menu *menu;
    Menuitem *mi;
    int i;

    menu = FindMenu("menu-select-scenario");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[9].flags&MenuButtonDisabled) {
		menu->items[9].flags &= ~MenuButtonDisabled;
		menu->items[9].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    ScenSelectLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

/**
**	Scenario select cancel button.
*/
local void ScenSelectCancel(void)
{
    char *s;

    //
    //  Use last selected map.
    //
    DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
    strcpy(ScenSelectPath, FreeCraftLibPath);
    if (*ScenSelectPath) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectPath, '/'))) {
	strcpy(ScenSelectFileName, s + 1);
	*s = '\0';
    }
    strcpy(ScenSelectDisplayPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
	*s = '\0';
    } else {
	*ScenSelectDisplayPath = '\0';
    }

    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

    EndMenu();
}

local void GameCancel(void)
{
    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;
    EndMenu();
}

/**
**	Custom game start game button pressed.
*/
local void CustomGameStart(void)
{
    int i;
    char *p;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
	p = ScenSelectPath + strlen(FreeCraftLibPath) + 1;
	strcpy(CurrentMapPath, p);
    } else {
	strcpy(CurrentMapPath, ScenSelectFileName);
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
    }

    for (i = 0; i < MAX_OBJECTIVES; i++) {
	if (GameIntro.Objectives[i]) {
	    free(GameIntro.Objectives[i]);
	    GameIntro.Objectives[i] = NULL;
	}
    }
    GameIntro.Objectives[0] = strdup(DefaultObjective);

    GuiGameStarted = 1;
    EndMenu();
}

/**
**	Single player custom game menu entered.
*/
local void GameSetupInit(Menuitem *mi __attribute__ ((unused)))
{
    char *s;

    //
    //  No old path, setup the default.
    //
    if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
#if 0					// FIXME: as soon as .cm is supported..
	strcpy(CurrentMapPath, "default.cm");
#endif
	strcpy(CurrentMapPath, "default.pud");

    }

    DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
    strcpy(ScenSelectPath, FreeCraftLibPath);
    if (*ScenSelectPath) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectPath, '/'))) {
	strcpy(ScenSelectFileName, s + 1);
	*s = '\0';
    }
    strcpy(ScenSelectDisplayPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
	*s = '\0';
    } else {
	*ScenSelectDisplayPath = '\0';
    }
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

    GetInfoFromSelectPath();
}

local void GameDrawFunc(Menuitem *mi __attribute__((unused)))
{
    int nc, rc, l;
    char buffer[32];

    GetDefaultTextColors(&nc, &rc);
    MenusSetBackground();
    SetDefaultTextColors(rc, rc);
    l = VideoTextLength(GameFont, "Scenario:");
    VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 360, GameFont, "Scenario:");
    VideoDrawText(TheUI.Offset640X + 16, TheUI.Offset480Y + 360+24 , GameFont, ScenSelectFileName);
    if (ScenSelectPudInfo) {
	if (ScenSelectPudInfo->Description) {
	    VideoDrawText(TheUI.Offset640X + 16 + l + 8, TheUI.Offset480Y + 360, GameFont, ScenSelectPudInfo->Description);
	}
	sprintf(buffer, " (%d x %d)", ScenSelectPudInfo->MapWidth, ScenSelectPudInfo->MapHeight);
	VideoDrawText(TheUI.Offset640X + 16+l+8+VideoTextLength(GameFont, ScenSelectFileName), TheUI.Offset480Y + 360+24, GameFont, buffer);
    }
#if 0
    for (n = j = 0; j < PlayerMax; j++) {
	if (info->PlayerType[j] == PlayerPerson) {
	    n++;
	}
    }
    if (n == 1) {
	VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,"1 player");
    } else {
	sprintf(buffer, "%d players", n);
	VideoDrawText(menu->x+8,menu->y+254+40,LargeFont,buffer);
    }
#endif
    SetDefaultTextColors(nc, rc);
}

/**
**	Menu setup race pulldown action.
**
**	@note FIXME: Todo support more and other races.
*/
local void GameRCSAction(Menuitem *mi, int i)
{
    int v[] = { PlayerRaceHuman, PlayerRaceOrc, SettingsPresetMapDefault };

    if (mi->d.pulldown.curopt == i) {
	GameSettings.Presets[0].Race = v[i];
	ServerSetupState.Race[0] = 2 - i;
	NetworkServerResyncClients();
    }
}

local void GameRESAction(Menuitem *mi, int i)
{
    int v[] = { SettingsResourcesMapDefault, SettingsResourcesLow,
		SettingsResourcesMedium, SettingsResourcesHigh };

    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.Resources = v[i];
	ServerSetupState.ResOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void GameUNSAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.NumUnits = i ? SettingsNumUnits1 : SettingsNumUnitsMapDefault;
	ServerSetupState.UnsOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void GameTSSAction(Menuitem *mi, int i)
{
    // FIXME: TilesetSummer, ... shouldn't be used, they will be removed.
    int v[] = { SettingsPresetMapDefault, TilesetSummer, TilesetWinter, TilesetWasteland, TilesetSwamp };

    if (!mi || mi->d.pulldown.curopt == i) {
	GameSettings.Terrain = v[i];
	ServerSetupState.TssOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

/**
**	Called if the pulldown menu of the game type is changed.
*/
local void GameGATAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	// FIXME: not supported
	// GameSettings.GameType = i-1;
	// ServerSetupState.GaTOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

local void CustomGameOPSAction(Menuitem *mi __attribute__((unused)), int i)
{
    GameSettings.Opponents = i ? i : SettingsPresetMapDefault;
}

/**
**	Menu setup fog-of-war pulldown action.
**
**	@note FIXME: Want to support more features. fe. reveal map on start.
*/
local void MultiGameFWSAction(Menuitem *mi, int i)
{
    if (!mi || mi->d.pulldown.curopt == i) {
	FlagRevealMap = i;
	ServerSetupState.FwsOpt = i;
	if (mi) {
	    NetworkServerResyncClients();
	}
    }
}

/**
**	Multiplayer menu player server states.
*/
local void MultiGamePTSAction(Menuitem *mi, int o)
{
    Menu *menu;
    int i;

    menu = FindMenu("menu-multi-setup");
    i = mi - menu->items - SERVER_PLAYER_STATE;
    // JOHNS: Must this be always true?
    // ARI: NO! think of client menus!
    // DebugCheck( i<0 || i>PlayerMax-1 );

    if (i > 0 && i < PlayerMax-1) {
	if (mi->d.pulldown.curopt == o) {
	    if (mi->d.pulldown.noptions == 2) {	// computer slot
		ServerSetupState.CompOpt[i] = o + 1;
	    } else {
		ServerSetupState.CompOpt[i] = o;
	    }
	    MultiGamePlayerSelectorsUpdate(3);	// Recalc buttons on server
	    NetworkServerResyncClients();
	}
    }
}

local void MultiGameDrawFunc(Menuitem *mi)
{
    GameDrawFunc(mi);
}

local void MultiGameClientDrawFunc(Menuitem *mi)
{
    // FIXME: do something better
    GameDrawFunc(mi);
}

/**
**	Multiplayer network game final race an player type setup.
*/
local void NetworkGamePrepareGameSettings(void)
{
    int c, h, i;
    int num[PlayerMax], comp[PlayerMax];

    DebugCheck(!ScenSelectPudInfo);

    DebugLevel0Fn("NetPlayers = %d\n" _C_ NetPlayers);

#ifdef DEBUG
    for (i = 0; i < PlayerMax-1; i++) {
	printf("%02d: CO: %d   Race: %d   Host: ", i, ServerSetupState.CompOpt[i], ServerSetupState.Race[i]);
	if (ServerSetupState.CompOpt[i] == 0) {
	    for (h = 0; h < NetPlayers; h++) {
		if (Hosts[h].PlyNr == i) {
		    printf("%s", Hosts[h].PlyName);
		}
	    }
	}
	printf("\n");
    }
#endif

    // Make a list of the available player slots.
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    DebugLevel3Fn("Player slot %i is available for a person\n" _C_ i);
	    num[h++] = i;
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    comp[c++] = i;	// available computer player slots
	}
    }
    for (i = 0; i < h; i++) {
	switch(ServerSetupState.CompOpt[num[i]]) {
	    case 0:
		GameSettings.Presets[num[i]].Type = PlayerPerson;
		DebugLevel3Fn("Settings[%d].Type == Person\n" _C_ num[i]);
		switch (ServerSetupState.Race[num[i]]) {
		    case 1:
			GameSettings.Presets[num[i]].Race = PlayerRaceOrc;
			break;
		    case 2:
			GameSettings.Presets[num[i]].Race = PlayerRaceHuman;
		    default:
			break;
		}
		break;
	    case 1:
		GameSettings.Presets[num[i]].Type = PlayerComputer;
		DebugLevel3Fn("Settings[%d].Type == Computer\n" _C_ num[i]);
		break;
	    case 2:
		GameSettings.Presets[num[i]].Type = PlayerNobody;
		DebugLevel3Fn("Settings[%d].Type == Closed\n" _C_ num[i]);
	    default:
		break;
	}
    }
    for (i = 0; i < c; i++) {
	if (ServerSetupState.CompOpt[comp[i]] == 2) {	// closed..
	    GameSettings.Presets[comp[i]].Type = PlayerNobody;
	    DebugLevel0Fn("Settings[%d].Type == Closed\n" _C_ comp[i]);
	}
    }

#ifdef DEBUG
    for (i = 0; i < NetPlayers; i++) {
	DebugCheck(GameSettings.Presets[Hosts[i].PlyNr].Type != PlayerPerson);
	;
    }
#endif
}

/**
**	Player selectors have changed.
**	Caution: Called by map change (initial = 1)!
**	Caution: Called by network events from clients (initial = 2)!
**	Caution: Called by button action on server (initial = 3)!
*/
local void MultiGamePlayerSelectorsUpdate(int initial)
{
    Menu *menu;
    int i, h, c;
    int avail, ready, plyrs;

    menu = FindMenu("menu-multi-setup");

    //	FIXME: What this has to do:
    //	Use lag gem as KICK button
    //  Notify clients about MAP change: (initial = 1...)

    DebugLevel3Fn("initial = %d\n" _C_ initial);

    //	Calculate available slots from pudinfo
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;	// available interactive player slots
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    c++;	// available computer player slots
	}
    }

    avail = h;
    plyrs = 0;
    //	Setup the player menu
    for (ready = i = 1; i < PlayerMax-1; i++) {
	if (initial == 1) {
	    if (i < h) {
		ServerSetupState.CompOpt[i] = 0;
	    }
	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;

	    // FIXME: don't forget to throw out additional players
	    //	  without available slots here!

	}
	if (Hosts[i].PlyNr) {
	    menu->items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[1];

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	    ++plyrs;
	    if (ServerSetupState.Ready[i]) {
		menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state |= MI_GSTATE_CHECKED;
		++ready;
	    }

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = 0;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_PASSIVE;
	} else {
	    // don't allow network and button events to intercept server player's action on pulldown buttons!
	    if (!(menu->items[SERVER_PLAYER_STATE + i].flags&MenuButtonClicked)) {
		if (initial == 1 ||
		    (initial == 2 && menu->items[SERVER_PLAYER_STATE + i].mitype != MI_TYPE_PULLDOWN)) {
		    menu->items[SERVER_PLAYER_STATE + i] = NetMultiButtonStorage[0];
		    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
		    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i];
		}
	    }
	    if (i < h && ServerSetupState.CompOpt[i] != 0) {
		avail--;
	    }

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	menu->items[SERVER_PLAYER_STATE + i].yofs = 32 + (i&7) * 22;
	if (i > 7) {
	    menu->items[SERVER_PLAYER_STATE + i].xofs = 320 + 40;
	}


	if (i >= h) {
	    // Allow to switch off (close) preset ai-computer slots
	    // FIXME: evaluate game type...
	    if (initial == 1 && i < h + c) {
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = 0;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.noptions = 2;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.options = mgptsoptions + 1;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 0;
		ServerSetupState.CompOpt[i] = 1;
		menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = ServerSetupState.CompOpt[i] - 1;
	    }

	    menu->items[SERVER_PLAYER_READY - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_READY - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;

	    menu->items[SERVER_PLAYER_LAG - 1 + i].flags = MenuButtonDisabled;
	    menu->items[SERVER_PLAYER_LAG - 1 + i].d.gem.state = MI_GSTATE_INVISIBLE;
	}

	if (i >= h + c) {
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.state = MI_PSTATE_PASSIVE;
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.defopt = 2;
	    menu->items[SERVER_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    menu->items[SERVER_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }


    //	Tell connect state machines how many interactive players we can have
    NetPlayers = avail;
    //	Check if all players are ready.
    DebugLevel0Fn("READY to START: AVAIL = %d, READY = %d\n" _C_ avail
	    _C_ ready);

    // Disable the select scenario after players have joined.
    if (plyrs) {
	// disable Select Scenario button
	menu->items[2].flags = MenuButtonDisabled;
    } else {
	// enable Select Scenario button
	menu->items[2].flags = 0;
    }
    if (ready == avail) {
	if (menu->items[3].flags == MenuButtonDisabled) {
	    // enable start game button
	    menu->items[3].flags = 0;
	}
    } else {
	// disable start game button
	menu->items[3].flags = MenuButtonDisabled;
    }
}

/**
**	Update client network menu.
*/
local void MultiClientUpdate(int initial)
{
    Menu *menu;
    int i;
    int h;
    int c;

    menu = FindMenu("menu-net-multi-client");

    //  Calculate available slots from pudinfo
    for (c = h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;			// available interactive player slots
	}
	if (ScenSelectPudInfo->PlayerType[i] == PlayerComputer) {
	    c++;			// available computer player slots
	}
    }

    //
    //	Setup defaults, reset values.
    //
    if (initial) {
	menu->items[CLIENT_PLAYER_STATE] = NetMultiButtonStorage[1];
	menu->items[CLIENT_PLAYER_STATE].yofs = 32;
	memset(&ServerSetupState, 0, sizeof(ServerSetup));
	memset(&LocalSetupState, 0, sizeof(ServerSetup));
    }
    for (i = 1; i < PlayerMax - 1; i++) {
	DebugLevel3Fn("%d: %d %d\n" _C_ i _C_ Hosts[i].PlyNr
		_C_ NetLocalHostsSlot);
	//
	//	Johns: This works only if initial. Hosts[i].PlyNr is later lost.
	//
	if (Hosts[i].PlyNr || i == NetLocalHostsSlot) {
	    menu->items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[1];
	    if (i == NetLocalHostsSlot) {
		menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state = 0;
	    } else {
		menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
		    MI_GSTATE_PASSIVE;
	    }
	} else {
	    menu->items[CLIENT_PLAYER_STATE + i] = NetMultiButtonStorage[0];
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.state =
		MI_PSTATE_PASSIVE;
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt =
		ServerSetupState.CompOpt[i];
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
		MI_GSTATE_INVISIBLE;
	}
	menu->items[CLIENT_PLAYER_STATE + i].yofs = 32 + (i & 7) * 22;
	if (i > 7) {
	    menu->items[CLIENT_PLAYER_STATE + i].xofs = 320 + 40;
	}
	menu->items[CLIENT_PLAYER_READY - 1 + i].flags = 0;

	if (ServerSetupState.Ready[i]) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state |=
		MI_GSTATE_CHECKED;
	} else {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state &=
		~MI_GSTATE_CHECKED;
	}

#if 0
	if (i != NetLocalHostsSlot) {
	//if (i >= h) {
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt =
		ServerSetupState.CompOpt[i];
	}
#endif

	// Unused slots are always disabled.
	if (i >= h + c) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].flags =
		MenuButtonDisabled;
	    menu->items[CLIENT_PLAYER_READY - 1 + i].d.gem.state =
		MI_GSTATE_INVISIBLE;
	    menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.defopt =
		menu->items[CLIENT_PLAYER_STATE + i].d.pulldown.curopt = 2;
	    menu->items[CLIENT_PLAYER_STATE + i].flags = MenuButtonDisabled;
	}
    }
}

local void MultiGameSetupInit(Menuitem *mi)
{
    Menu *menu;
    int i, h;

#ifdef OLD_MENU
    menu = FindMenu("menu-multi-setup");
#else
    menu = mi->menu;
#endif

    // FIXME: Remove this when .cm is supported
    if (*CurrentMapPath && strstr(CurrentMapPath, ".cm\0")) {
	*CurrentMapPath='\0';
    }

    GameSetupInit(mi);
    NetworkInitServerConnect();
    menu->items[SERVER_PLAYER_STATE] = NetMultiButtonStorage[1];
    menu->items[SERVER_PLAYER_STATE].yofs = 32;

    memset(&ServerSetupState, 0, sizeof(ServerSetup));
    //	Calculate available slots from pudinfo
    for (h = i = 0; i < PlayerMax; i++) {
	if (ScenSelectPudInfo->PlayerType[i] == PlayerPerson) {
	    h++;	// available interactive player slots
	}
    }
    for (i = h; i < PlayerMax - 1; i++) {
	ServerSetupState.CompOpt[i] = 1;
    }
    MultiGamePlayerSelectorsUpdate(1);
    DebugLevel3Fn("h = %d, NetPlayers = %d\n" _C_ h _C_ NetPlayers);
}

local void MultiGameSetupExit(Menuitem *mi __attribute__((unused)))
{
    NetworkExitServerConnect();
}

/**
**	Cancel button of server multi player menu pressed.
*/
local void MultiGameCancel(void)
{
    MultiGameSetupExit(NULL);
    NetPlayers = 0;		// Make single player menus work again!
    GameCancel();
}

/**
**	Draw the multi player setup menu.
*/
local void NetMultiPlayerDrawFunc(Menuitem *mi)
{
    Menu *menu;
    int i, nc, rc;

    menu = FindMenu("menu-multi-setup");
    i = mi - menu->items - SERVER_PLAYER_STATE;
    if (i >= 0 && i < PlayerMax - 1) {		// Ugly test to detect server
	if (i > 0) {
	    menu->items[SERVER_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    // Note: re-disabled in MultiGamePlayerSelectorsUpdate()
	    //		for kicked out clients!!
	    if (ServerSetupState.Ready[i]) {
		menu->items[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		menu->items[SERVER_PLAYER_READY - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }
	    if (ServerSetupState.LastFrame[i] + 30 > FrameCounter) {
		menu->items[SERVER_PLAYER_LAG - 1 + i].flags &=
		    ~MenuButtonDisabled;
		menu->items[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE|MI_GSTATE_CHECKED;
	    } else {
		menu->items[SERVER_PLAYER_LAG - 1 + i].flags |=
		    MenuButtonDisabled;
		menu->items[SERVER_PLAYER_LAG - 1 + i]
			.d.gem.state = MI_GSTATE_PASSIVE;
	    }

	}
    } else {
	menu = FindMenu("menu-net-multi-client");
	i = mi - menu->items - CLIENT_PLAYER_STATE;
	if (i > 0) {
	    menu->items[CLIENT_PLAYER_READY - 1 + i].flags &=
		~MenuButtonDisabled;
	    if (i == NetLocalHostsSlot) {
		menu->items[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state &= ~MI_GSTATE_PASSIVE;
	    } else {
		menu->items[CLIENT_PLAYER_READY - 1 + i].
			d.gem.state |= MI_GSTATE_PASSIVE;
	    }
	}
    }

    GetDefaultTextColors(&nc, &rc);
    SetDefaultTextColors(rc, rc);
    DebugLevel3Fn("Hosts[%d].PlyName = %s\n" _C_ i _C_ Hosts[i].PlyName);
    VideoDrawText(TheUI.Offset640X+mi->xofs, TheUI.Offset480Y+mi->yofs, GameFont, Hosts[i].PlyName);

    SetDefaultTextColors(nc, rc);
}

/**
**	Cancel button of multiplayer client menu pressed.
*/
local void MultiClientCancel(void)
{
    NetworkDetachFromServer();
    // GameCancel();
}

#ifdef OLD_MENU
local void MultiGameClientInit(Menuitem *mi __attribute__((unused)))
#else
local void MultiGameClientInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-net-multi-client");
#else
    menu = mi->menu;
#endif

    // GameSetupInit(mi);
    MultiClientUpdate(1);
    if (LocalSetupState.Ready[NetLocalHostsSlot]) {
	menu->items[2].flags = MenuButtonDisabled;
	menu->items[3].flags = 0;
    } else {
	menu->items[3].flags = MenuButtonDisabled;
	menu->items[2].flags = 0;
    }
}

/**
**	Multiplayer client gem action. Toggles ready flag.
*/
local void MultiClientGemAction(Menuitem *mi)
{
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-net-multi-client");
#else
    menu = mi->menu;
#endif
    i = mi - menu->items - CLIENT_PLAYER_READY + 1;
    DebugLevel3Fn("i = %d, NetLocalHostsSlot = %d\n" _C_ i _C_ NetLocalHostsSlot);
    if (i == NetLocalHostsSlot) {
	LocalSetupState.Ready[i] = !LocalSetupState.Ready[i];
	if (LocalSetupState.Ready[i]) {
	    menu->items[2].flags = MenuButtonDisabled;
	    menu->items[3].flags = 0;
	} else {
	    menu->items[3].flags = MenuButtonDisabled;
	    menu->items[2].flags = 0;
	}
	MultiClientUpdate(0);
    }
}

local void MultiClientRCSAction(Menuitem *mi, int i)
{
    if (mi->d.pulldown.curopt == i) {
	LocalSetupState.Race[NetLocalHostsSlot] = 2 - i;
	MultiClientUpdate(0);
    }
}

local void MultiClientReady(void)
{
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");
    menu->items[2].flags = MenuButtonDisabled;
    menu->items[3].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 1;
    MultiClientUpdate(0);
}

local void MultiClientNotReady(void)
{
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");
    menu->items[3].flags = MenuButtonDisabled;
    menu->items[2].flags = 0;
    LocalSetupState.Ready[NetLocalHostsSlot] = 0;
    MultiClientUpdate(0);
}

/**
**	Callback from netconnect loop in Client-Sync state:
**	Compare local state with server's information
**	and force update when changes have occured.
*/
global void NetClientCheckLocalState(void)
{
    if (LocalSetupState.Ready[NetLocalHostsSlot] != ServerSetupState.Ready[NetLocalHostsSlot]) {
	NetLocalState = ccs_changed;
	return;
    }
    if (LocalSetupState.Race[NetLocalHostsSlot] != ServerSetupState.Race[NetLocalHostsSlot]) {
	NetLocalState = ccs_changed;
	return;
    }
    /* ADD HERE */
}

global int NetClientSelectScenario(void)
{
    char *cp;

    FreeMapInfo(ScenSelectPudInfo);
    ScenSelectPudInfo = NULL;

    cp = strrchr(ScenSelectFullPath, '/');
    if (cp) {
	strcpy(ScenSelectFileName, cp + 1);
	*cp = 0;
	strcpy(ScenSelectPath, ScenSelectFullPath);
	*cp = '/';
    } else {
	strcpy(ScenSelectFileName, ScenSelectFullPath);
	ScenSelectPath[0] = 0;
    }

    if (strcasestr(ScenSelectFileName, ".pud")) {
	ScenSelectPudInfo = GetPudInfo(ScenSelectFullPath);
    } else {
	// FIXME: GetCmInfo();
    }
    return ScenSelectPudInfo == NULL;
}

global void NetConnectForceDisplayUpdate(void)
{
    MultiGamePlayerSelectorsUpdate(2);
    MustRedraw |= RedrawMenu;
}

/**
**	Update client menu to follow server menu.
*/
global void NetClientUpdateState(void)
{
    Menu *menu;

    menu = FindMenu("menu-net-multi-client");

    GameRESAction(NULL, ServerSetupState.ResOpt);
    menu->items[CLIENT_RESOURCE].d.pulldown.curopt =
	ServerSetupState.ResOpt;

    GameUNSAction(NULL, ServerSetupState.UnsOpt);
    menu->items[CLIENT_UNITS].d.pulldown.curopt =
	ServerSetupState.UnsOpt;

    MultiGameFWSAction(NULL, ServerSetupState.FwsOpt);
    menu->items[CLIENT_FOG_OF_WAR].d.pulldown.curopt =
	ServerSetupState.FwsOpt;

    GameTSSAction(NULL, ServerSetupState.TssOpt);
    menu->items[CLIENT_TILESET].d.pulldown.curopt =
	ServerSetupState.TssOpt;

    MultiClientUpdate(0);
    DebugLevel1Fn("MultiClientMenuRedraw\n");

    MustRedraw |= RedrawMenu;
}

/**
**	Start editor.
*/
local void StartEditor(void)
{
    char* s;

    VideoLockScreen();
    MenusSetBackground();
    VideoUnlockScreen();
    Invalidate();

    if (!*CurrentMapPath || *CurrentMapPath == '.' || *CurrentMapPath == '/') {
	strcpy(CurrentMapPath, "default.pud");
    }

    strcpy(ScenSelectPath, FreeCraftLibPath);
    if (*ScenSelectPath) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectPath, '/'))) {
	strcpy(ScenSelectFileName, s + 1);
	*s = '\0';
    }
    strcpy(ScenSelectDisplayPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
	*s = '\0';
    } else {
	*ScenSelectDisplayPath = '\0';
    }

    GetInfoFromSelectPath();

    ProcessMenu("menu-editor-select", 1);
}

local void EditorNewMap(void)
{
    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    // FIXME: currently just loads default.pud
    strcpy(CurrentMapPath, "default.pud");

    // FIXME: Use EditorRunning and main-loop.
    EditorMainLoop();
    EndMenu();
}

local void EditorLoadMap(void)
{
    char *p;

    EditorLoadCancelled=0;
    ProcessMenu("menu-editor-load-map", 1);
    GetInfoFromSelectPath();

    if (EditorLoadCancelled) {
	VideoLockScreen();
	MenusSetBackground();
	VideoUnlockScreen();
	return;
    }

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    if (ScenSelectPath[0]) {
	strcat(ScenSelectPath, "/");
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
	p = ScenSelectPath + strlen(FreeCraftLibPath) + 1;
	strcpy(CurrentMapPath, p);
    } else {
	strcpy(CurrentMapPath, ScenSelectFileName);
	strcat(ScenSelectPath, ScenSelectFileName);	// Final map name with path
    }

    // FIXME: Use EditorRunning and main-loop.
    EditorMainLoop();
    EndMenu();
}

#ifdef OLD_MENU
local void EditorLoadInit(Menuitem *mi __attribute__((unused)))
#else
local void EditorLoadInit(Menuitem *mi)
#endif
{
    Menu *menu;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    DebugCheck(!*ScenSelectPath);
    menu->items[5].flags =
	*ScenSelectDisplayPath ? 0 : MenuButtonDisabled;
    menu->items[5].d.button.text = ScenSelectDisplayPath;
    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);
}

local void EditorLoadLBInit(Menuitem *mi)
{
    Menu *menu;
    int i;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    EditorLoadLBExit(mi);
    i = mi->d.listbox.noptions = ReadDataDirectory(ScenSelectPath, EditorLoadRDFilter,
	(FileList **)&(mi->d.listbox.options));

    if (i == 0) {
	menu->items[3].d.button.text = "OK";
	menu->items[3].flags |= MenuButtonDisabled;
    } else {
	EditorLoadLBAction(mi, 0);
	menu->items[3].flags &= ~MenuButtonDisabled;
	if (i > 5) {
	    mi[1].flags &= ~MenuButtonDisabled;
	}
    }
}

local void EditorLoadLBExit(Menuitem *mi)
{
    FileList *fl;

    if (mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	FreeMapInfos(fl, mi->d.listbox.noptions);
	free(fl);
	mi->d.listbox.options = NULL;
	mi->d.listbox.noptions = 0;
	mi[1].flags |= MenuButtonDisabled;
    }
}

local int EditorLoadRDFilter(char *pathbuf, FileList *fl)
{
    MapInfo *info;
    char *suf;
    char *np, *cp, *lcp;
#ifdef USE_ZZIPLIB
    int sz;
    ZZIP_FILE *zzf;
#endif

    suf = ".pud";
    np = strrchr(pathbuf, '/');
    if (np) {
	np++;
    } else {
	np = pathbuf;
    }
    cp = np;
    cp--;
    fl->type = -1;
#ifdef USE_ZZIPLIB
    if ((zzf = zzip_open(pathbuf, O_RDONLY|O_BINARY))) {
	sz = zzip_file_real(zzf);
	zzip_close(zzf);
	if (!sz) {
	    goto usezzf;
	}
    }
#endif
    do {
	lcp = cp++;
	cp = strcasestr(cp, suf);
    } while (cp != NULL);
    if (lcp >= np) {
	cp = lcp + strlen(suf);
#ifdef USE_ZLIB
	if (strcmp(cp, ".gz") == 0) {
	    *cp = 0;
	}
#endif
#ifdef USE_BZ2LIB
	if (strcmp(cp, ".bz2") == 0) {
	    *cp = 0;
	}
#endif
	if (*cp == 0) {
#ifdef USE_ZZIPLIB
usezzf:
#endif
	    if (strcasestr(pathbuf, ".pud")) {
		info = GetPudInfo(pathbuf);
		if (info) {
		    fl->type = 1;
		    fl->name = strdup(np);
		    fl->xdata = info;
		    return 1;
		}
	    }
	}
    }
    return 0;
}

local void EditorLoadFolder(void)
{
    Menu *menu;
    Menuitem *mi;
    char *cp;

    menu = FindMenu("menu-editor-load-map");
    mi = &menu->items[1];

    if (ScenSelectDisplayPath[0]) {
	cp = strrchr(ScenSelectDisplayPath, '/');
	if (cp) {
	    *cp = 0;
	} else {
	    ScenSelectDisplayPath[0] = 0;
	    menu->items[5].flags |= MenuButtonDisabled;
	    menu->items[5].d.button.text = NULL;
	}
	cp = strrchr(ScenSelectPath, '/');
	if (cp) {
	    *cp = 0;
	    EditorLoadLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	}
    }
}

local void EditorLoadOk(void)
{
    Menu *menu;
    Menuitem *mi;
    FileList *fl;
    int i;

    menu = FindMenu("menu-editor-load-map");
    mi = &menu->items[1];
    i = mi->d.listbox.curopt + mi->d.listbox.startline;
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type == 0) {
	    strcat(ScenSelectPath, "/");
	    strcat(ScenSelectPath, fl[i].name);
	    if (menu->items[5].flags&MenuButtonDisabled) {
		menu->items[5].flags &= ~MenuButtonDisabled;
		menu->items[5].d.button.text = ScenSelectDisplayPath;
	    } else {
		strcat(ScenSelectDisplayPath, "/");
	    }
	    strcat(ScenSelectDisplayPath, fl[i].name);
	    EditorLoadLBInit(mi);
	    mi->d.listbox.cursel = -1;
	    mi->d.listbox.startline = 0;
	    mi->d.listbox.curopt = 0;
	    mi[1].d.vslider.percent = 0;
	    mi[1].d.hslider.percent = 0;
	    MustRedraw |= RedrawMenu;
	} else {
	    strcpy(ScenSelectFileName, fl[i].name);	// Final map name
	    EndMenu();
	}
    }
}

local void EditorLoadCancel(void)
{
    char* s;

    EditorLoadCancelled=1;

    //
    //  Use last selected map.
    //
    DebugLevel0Fn("Map   path: %s\n" _C_ CurrentMapPath);
    strcpy(ScenSelectPath, FreeCraftLibPath);
    if (*ScenSelectPath) {
	strcat(ScenSelectPath, "/");
    }
    strcat(ScenSelectPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectPath, '/'))) {
	strcpy(ScenSelectFileName, s + 1);
	*s = '\0';
    }
    strcpy(ScenSelectDisplayPath, CurrentMapPath);
    if ((s = strrchr(ScenSelectDisplayPath, '/'))) {
	*s = '\0';
    } else {
	*ScenSelectDisplayPath = '\0';
    }

    DebugLevel0Fn("Start path: %s\n" _C_ ScenSelectPath);

    EndMenu();
}

local unsigned char *EditorLoadLBRetrieve(Menuitem *mi, int i)
{
    FileList *fl;
    Menu *menu;
    MapInfo *info;
    static char buffer[1024];
    int j, n;

    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    if (i - mi->d.listbox.startline == mi->d.listbox.curopt) {
		if ((info = fl[i].xdata)) {
#ifdef OLD_MENU
		    menu = FindMenu("menu-editor-load-map");
#else
		    menu = mi->menu;
#endif
		    if (info->Description) {
			VideoDrawText(menu->x+8,menu->y+234,LargeFont,info->Description);
		    }
		    sprintf(buffer, "%d x %d", info->MapWidth, info->MapHeight);
		    VideoDrawText(menu->x+8,menu->y+234+20,LargeFont,buffer);
		    for (n = j = 0; j < PlayerMax; j++) {
			if (info->PlayerType[j] == PlayerPerson) {
			    n++;
			}
		    }
		    if (n == 1) {
			VideoDrawText(menu->x+8,menu->y+234+40,LargeFont,"1 player");
		    } else {
			sprintf(buffer, "%d players", n);
			VideoDrawText(menu->x+8,menu->y+234+40,LargeFont,buffer);
		    }
		}
	    }
	    strcpy(buffer, "   ");
	} else {
	    strcpy(buffer, "\260 ");
	}
	strcat(buffer, fl[i].name);
	return buffer;
    }
    return NULL;
}

local void EditorLoadLBAction(Menuitem *mi, int i)
{
    Menu *menu;
    FileList *fl;

#ifdef OLD_MENU
    menu = FindMenu("menu-editor-load-map");
#else
    menu = mi->menu;
#endif
    DebugCheck(i<0);
    if (i < mi->d.listbox.noptions) {
	fl = mi->d.listbox.options;
	if (fl[i].type) {
	    menu->items[3].d.button.text = "OK";
	} else {
	    menu->items[3].d.button.text = "Open";
	}
	if (mi->d.listbox.noptions > 5) {
	    mi[1].d.vslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	    mi[1].d.hslider.percent = (i * 100) / (mi->d.listbox.noptions - 1);
	}
    }
}

local void EditorLoadVSAction(Menuitem *mi, int i)
{
    int op, d1, d2;

    mi--;
    switch (i) {
	case 0:		// click - down
	case 2:		// key - down
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_DOWN) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
		    mi->d.listbox.curopt++;
		    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
			mi->d.listbox.curopt--;
			mi->d.listbox.startline++;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    } else if (mi[1].d.vslider.cflags&MI_CFLAGS_UP) {
		if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
		    mi->d.listbox.curopt--;
		    if (mi->d.listbox.curopt < 0) {
			mi->d.listbox.curopt++;
			mi->d.listbox.startline--;
		    }
		    MustRedraw |= RedrawMenu;
		}
	    }
	    EditorLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
	    if (i == 2) {
		mi[1].d.vslider.cflags &= ~(MI_CFLAGS_DOWN|MI_CFLAGS_UP);
	    }
	    break;
	case 1:		// mouse - move
	    if (mi[1].d.vslider.cflags&MI_CFLAGS_KNOB && (mi[1].flags&MenuButtonClicked)) {
		if (mi[1].d.vslider.curper > mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.listbox.noptions) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline + 1) * 100) /
				 (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.curper - mi[1].d.vslider.percent;
			    d2 = op - mi[1].d.vslider.curper;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt++;
			    if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
				mi->d.listbox.curopt--;
				mi->d.listbox.startline++;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline+1 == mi->d.listbox.noptions)
				break;
			}
		    }
		} else if (mi[1].d.vslider.curper < mi[1].d.vslider.percent) {
		    if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
			for (;;) {
			    op = ((mi->d.listbox.curopt + mi->d.listbox.startline - 1) * 100) /
				     (mi->d.listbox.noptions - 1);
			    d1 = mi[1].d.vslider.percent - mi[1].d.vslider.curper;
			    d2 = mi[1].d.vslider.curper - op;
			    if (d2 >= d1)
				break;
			    mi->d.listbox.curopt--;
			    if (mi->d.listbox.curopt < 0) {
				mi->d.listbox.curopt++;
				mi->d.listbox.startline--;
			    }
			    if (mi->d.listbox.curopt+mi->d.listbox.startline == 0)
				break;
			}
		    }
		}

		DebugCheck(mi->d.listbox.startline < 0);
		DebugCheck(mi->d.listbox.startline+mi->d.listbox.curopt >= mi->d.listbox.noptions);

		EditorLoadLBAction(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
		MustRedraw |= RedrawMenu;
	    }
	    break;
	default:
	    break;
    }
}

local void EditorMapProperties(void)
{
    Menu *menu;
    char MapDescription[36];
    char MapSize[30];

    menu = FindMenu("menu-editor-map-properties");

    menu->items[2].d.input.buffer = MapDescription;
    strcpy(MapDescription, TheMap.Info->Description);
    strcat(MapDescription, "~!_");
    menu->items[2].d.input.nch = strlen(MapDescription)-3;
    menu->items[2].d.input.maxch = 31;

    sprintf(MapSize, "%d x %d", TheMap.Width, TheMap.Height);
    menu->items[4].d.text.text = MapSize;

    menu->items[6].d.pulldown.defopt = TheMap.Terrain;

    // FIXME: Set the correct pud version
    menu->items[8].d.pulldown.defopt = 1;
    menu->items[8].flags = -1;

    ProcessMenu("menu-editor-map-properties", 1);
}

local void EditorEnterMapDescriptionAction(
	Menuitem *mi __attribute__((unused)), int key)
{
    if (key == 10 || key == 13) {
	EditorEndMenu();
    }
}

local void EditorMapPropertiesOk(void)
{
    Menu *menu;
    char *MapDescription;
    // FIXME: TilesetSummer, ... shouldn't be used, they will be removed.
    int v[] = { TilesetSummer, TilesetWinter, TilesetWasteland, TilesetSwamp };

    menu = FindMenu("menu-editor-map-properties");

    MapDescription = menu->items[2].d.input.buffer;
    MapDescription[strlen(MapDescription)-3] = '\0';
    strcpy(TheMap.Info->Description, MapDescription);

    // FIXME: Need to actually change the terrain
    TheMap.Terrain = v[menu->items[6].d.pulldown.curopt];

    // FIXME: Save the pud version somewhere

    EditorEndMenu();
}

local void EditorMapPropertiesCancel(void)
{
    EditorEndMenu();
}

local void EditorPlayerPropertiesDrawFunc(Menuitem *mi __attribute__((unused)))
{
    MenusSetBackground();
}

local void EditorPlayerPropertiesEnterAction(Menuitem *mi,
	int key __attribute__((unused)))
{
    if (mi->d.input.nch > 0 && !isdigit(mi->d.input.buffer[mi->d.input.nch-1])) {
	strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
    }
}

local int player_types_fc_to_menu[] = {
    0,
    0,
    4,
    5,
    1,
    0,
    2,
    3,
};
local int player_types_menu_to_fc[] = {
    PlayerPerson,
    PlayerComputer,
    PlayerRescuePassive,
    PlayerRescueActive,
    PlayerNeutral,
    PlayerNobody,
};

local int player_ai_fc_to_menu(int num)
{
    if (num == PlayerAiLand) {
	return 0;
    } else if (num == PlayerAiPassive) {
	return 1;
    } else if (num == PlayerAiSea) {
	return 2;
    } else if (num == PlayerAiAir) {
	return 3;
    }
    DebugLevel0Fn("Invalid Ai number: %d\n" _C_ num);
    return -1;
}
local int player_ai_menu_to_fc(int num)
{
    if (num == 0) {
	return PlayerAiLand;
    } else if (num == 1) {
	return PlayerAiPassive;
    } else if (num == 2) {
	return PlayerAiSea;
    } else if (num == 3) {
	return PlayerAiAir;
    }
    DebugLevel0Fn("Invalid Ai number: %d\n" _C_ num);
    return -1;
}

/**
**	Edit player properties menu
*/
local void EditorPlayerProperties(void)
{
    Menu *menu;
    char gold[16][15];
    char lumber[16][15];
    char oil[16][15];
    int i;

    menu = FindMenu("menu-editor-player-properties");

#define RACE_POSITION 21
#define TYPE_POSITION 38
#define AI_POSITION 55
#define GOLD_POSITION 72
#define LUMBER_POSITION 89
#define OIL_POSITION 106

    for (i=0; i<16; ++i) {
	menu->items[RACE_POSITION+i].d.pulldown.defopt = TheMap.Info->PlayerSide[i];
	menu->items[TYPE_POSITION+i].d.pulldown.defopt = player_types_fc_to_menu[TheMap.Info->PlayerType[i]];
	menu->items[AI_POSITION+i].d.pulldown.defopt = player_ai_fc_to_menu(TheMap.Info->PlayerAi[i]);
	sprintf(gold[i], "%d~!_", TheMap.Info->PlayerGold[i]);
	sprintf(lumber[i], "%d~!_", TheMap.Info->PlayerWood[i]);
	sprintf(oil[i], "%d~!_", TheMap.Info->PlayerOil[i]);
	menu->items[GOLD_POSITION+i].d.input.buffer = gold[i];
	menu->items[GOLD_POSITION+i].d.input.nch = strlen(gold[i]) - 3;
	menu->items[GOLD_POSITION+i].d.input.maxch = 7;
	menu->items[LUMBER_POSITION+i].d.input.buffer = lumber[i];
	menu->items[LUMBER_POSITION+i].d.input.nch = strlen(lumber[i]) - 3;
	menu->items[LUMBER_POSITION+i].d.input.maxch = 7;
	menu->items[OIL_POSITION+i].d.input.buffer = oil[i];
	menu->items[OIL_POSITION+i].d.input.nch = strlen(oil[i]) - 3;
	menu->items[OIL_POSITION+i].d.input.maxch = 7;
    }

    ProcessMenu("menu-editor-player-properties", 1);

    for (i=0; i<16; ++i) {
	TheMap.Info->PlayerSide[i] = menu->items[RACE_POSITION+i].d.pulldown.curopt;
	TheMap.Info->PlayerType[i] = player_types_menu_to_fc[menu->items[TYPE_POSITION+i].d.pulldown.curopt];
	TheMap.Info->PlayerAi[i] = player_ai_menu_to_fc(menu->items[AI_POSITION+i].d.pulldown.curopt);
	TheMap.Info->PlayerGold[i] = atoi(gold[i]);
	TheMap.Info->PlayerWood[i] = atoi(lumber[i]);
	TheMap.Info->PlayerOil[i] = atoi(oil[i]);
    }

    VideoCreatePalette(GlobalPalette);
}

/**
**	Edit resource properties
*/
global void EditorEditResource(void)
{
    Menu *menu;
    char buf[13];

    menu = FindMenu("menu-editor-edit-resource");

    if (UnitUnderCursor->Type->GoldMine) {
	menu->items[0].d.text.text = "Amount of gold:";
    } else if (UnitUnderCursor->Type->OilPatch || UnitUnderCursor->Type->GivesOil) {
	menu->items[0].d.text.text = "Amount of oil:";
    }
    sprintf(buf, "%d~!_", UnitUnderCursor->Value);
    menu->items[1].d.input.buffer = buf;
    menu->items[1].d.input.nch = strlen(buf) - 3;
    menu->items[1].d.input.maxch = 6;
    ProcessMenu("menu-editor-edit-resource", 1);
}

/**
**	Key pressed in menu-editor-edit-resource
*/
local void EditorEditResourceEnterAction(Menuitem *mi,int key)
{
    if (mi->d.input.nch > 0 && !isdigit(mi->d.input.buffer[mi->d.input.nch-1])) {
	strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
    } else if (key==10 || key==13) {
	EditorEditResourceOk();
    }
}

/**
**	Ok button from menu-editor-edit-resource
*/
local void EditorEditResourceOk(void)
{
    Menu *menu;
    unsigned value;

    menu = FindMenu("menu-editor-edit-resource");
    value = atoi(menu->items[1].d.input.buffer);
    if (value < 2500) {
	strcpy(menu->items[1].d.text.text, "2500~!_");
	menu->items[1].d.input.nch = strlen(menu->items[1].d.text.text) - 3;
	menu = FindMenu("menu-editor-error");
	menu->items[1].d.text.text = "Must be greater than 2500";
	ProcessMenu("menu-editor-error", 1);
    } else if (value > 655000) {
	strcpy(menu->items[1].d.text.text, "655000~!_");
	menu->items[1].d.input.nch = strlen(menu->items[1].d.text.text) - 3;
	menu = FindMenu("menu-editor-error");
	menu->items[1].d.text.text = "Must be smaller than 655000";
	ProcessMenu("menu-editor-error", 1);
    } else if (value/2500*2500 != value) {
	value = (value+1250)/2500*2500;
	sprintf(menu->items[1].d.text.text, "%d~!_", value);
	menu->items[1].d.input.nch = strlen(menu->items[1].d.text.text) - 3;
	menu = FindMenu("menu-editor-error");
	menu->items[1].d.text.text = "Must be a multiple of 2500";
	ProcessMenu("menu-editor-error", 1);
    } else {
	UnitUnderCursor->Value = value;
	GameMenuReturn();
    }
}

/**
**	Cancel button from menu-editor-edit-resource
*/
local void EditorEditResourceCancel(void)
{
    GameMenuReturn();
}

/**
**	Called from menu, to quit editor to menu.
**
**	@todo Should check if modified file should be saved.
*/
local void EditorQuitMenu(void)
{
    EditorRunning=0;
    GameMenuReturn();
}

/**
**	End menus state of the editor.
*/
local void EditorEndMenu(void)
{
    CursorOn = CursorOnUnknown;
    CurrentMenu = NULL;

    MustRedraw = RedrawEverything;
    InterfaceState = IfaceStateNormal;
    EditorUpdateDisplay();
    InterfaceState = IfaceStateMenu;
    MustRedraw = RedrawMenu;
}

/*----------------------------------------------------------------------------
--	Init functions
----------------------------------------------------------------------------*/

/**
**	Move buttons so they're centered on different resolutions
*/
#ifdef OLD_MENU
local void CenterMenus(void)
{
    int menus;
    int i;
    int oldOffsetX;
    int oldOffsetY;

    oldOffsetX = TheUI.Offset640X;
    oldOffsetY = TheUI.Offset480Y;
    TheUI.Offset640X = (VideoWidth - 640) / 2;
    TheUI.Offset480Y = (VideoHeight - 480) / 2;

    if (TheUI.Offset640X == 0 && oldOffsetX == 0) {
	return;
    }

    menus = sizeof(Menus) / sizeof(Menu);
    for (i=0; i<menus; i++) {
	Menus[i].x += TheUI.Offset640X - oldOffsetX;
	Menus[i].y += TheUI.Offset480Y - oldOffsetY;
    }
}
#endif

/**
**	Initialize the loaded menu data
*/
global void InitMenuData(void)
{
    Menu *menu;

#ifdef OLD_MENU
    InitOldMenuData();
#endif

    // FIXME: Get rid of these..
    menu = FindMenu("menu-net-connecting");
    menu->items[1].d.text.text = NetServerText;
    menu->items[2].d.text.text = NetTriesText;
    InitNetMultiButtonStorage();

#ifdef OLD_MENU
    CenterMenus();
#endif
}

/**
**	Post-Initialize the loaded menu functions
*/
global void InitMenuFunctions(void)
{
    Menu *menu;

    //
    //	Autodetect the swamp tileset
    //
#ifdef HAVE_EXPANSION
    strcpy(ScenSelectFullPath, FreeCraftLibPath);
    if (ScenSelectFullPath[0]) {
	strcat(ScenSelectFullPath, "/graphics/tilesets/");
    }
    strcat(ScenSelectFullPath, "swamp");
    menu = FindMenu("menu-custom-game");
    //
    //	FIXME: Johns: this didn't work if the files are in ZIP archive.
    //
    if (access(ScenSelectFullPath, F_OK) != 0) {
	// ARI FIXME: Hack to disable Expansion Gfx..
	// also shows how to add new tilesets....
	// - FIXME2:
	// With new dynamic tileset configuration this
	// should read what siod-config gave us and
	// build the menu from that..
	menu->items[14].d.pulldown.noptions = 4;
    }
#else
    //
    // HACK_MOUSE
    //
    //menu->items[14].d.pulldown.noptions = 4;
    //
#endif

#ifdef SAVE_MENU_CCL
    {
	FILE *fd=fopen("menus.ccl","wb");
	SaveMenus(fd);
	fclose(fd);
    }
#endif
}

//@}
