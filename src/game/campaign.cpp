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
/**@name campagin.c	-	The campaign control. */
//
//	(c) Copyright 2002 by Lutz Sammer
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freecraft.h"
#include "ccl.h"
#include "unittype.h"
#include "map.h"
#include "campaign.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int GameResult;			/// Outcome of the game
global Campaign* Campaigns;		/// Campaigns
global int NumCampaigns;		/// Number of campaigns

global Campaign* CurrentCampaign;	/// Playing this campaign
global CampaignChapter* CurrentChapter;	/// Playing this chapter of campaign

/**
**	Unit-type type definition
*/
global const char CampaignType[] = "campaign-type";

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Return filename of next chapter.
**
**	@return		The filename of the next level
*/
global char* NextChapter(void)
{
    if (!CurrentCampaign) {
	return NULL;
    }
    if (!CurrentChapter) {
	return NULL;
    }

    CurrentChapter->Result = GameResult;

    if (GameResult == GameVictory) {
	//
	//  FIXME: do other chapter types.
	//
	do {
	    CurrentChapter = CurrentChapter->Next;
	} while (CurrentChapter && CurrentChapter->Type != ChapterPlayLevel);
    }
    // FIXME: handle defeat

    if (!CurrentChapter) {
	return NULL;
    }

    return CurrentChapter->Name;
}

/**
**	Play the campaign.
**
**	@param ident	Name of the campaign.
*/
global void PlayCampaign(const char* name)
{
    char* s;
    char* filename;
    int i;

    //
    //  Find the campaign.
    //
    for (i = 0; i < NumCampaigns; ++i) {
	if (!strcmp(Campaigns[i].Ident, name)) {
	    CurrentCampaign = Campaigns + i;
	}
    }
    if (!CurrentCampaign) {
	return;
    }
    CurrentChapter = CurrentCampaign->Chapters;

    //
    //  FIXME: do other chapter types.
    //
    while (CurrentChapter && CurrentChapter->Type != ChapterPlayLevel) {
	CurrentChapter = CurrentChapter->Next;
    }

    DebugCheck(!CurrentChapter);

    // FIXME: Johns is this here needed? Can the map loaded in create game?
    // ARI: Yes - This switches the menu gfx.. from def. Orc to Human, etc
    InitUnitTypes();
    UpdateStats();
    // ARI: And this finally loads it.
    //  For an alternative Method see network games..
    //  That way it should work finally..

    filename = CurrentChapter->Name;
    s = NULL;
    // FIXME: LibraryFile here?
    if (filename[0] != '/' && filename[0] != '.') {
	s = filename = strdcat3(FreeCraftLibPath, "/", filename);
    }
    //
    //  Load the map.
    //
    InitUnitTypes();
    UpdateStats();
    LoadMap(filename, &TheMap);

    if (s) {
	free(s);
    }
}

/**
**	Define a campaign.
**
**	@param list	List describing the campaign.
*/
local SCM CclDefineCampaign(SCM list)
{
    char* ident;
    SCM value;
    SCM sublist;
    Campaign* campaign;
    CampaignChapter* chapter;
    CampaignChapter** tail;

    DebugLevel0Fn("Not written\n");

    //
    //	Campaign name
    //
    ident=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    // FIXME: must overwrite old campaign with same ident
    if( Campaigns ) {
	Campaigns=realloc(Campaigns,sizeof(Campaign)*(NumCampaigns+1));
	campaign=Campaigns+NumCampaigns;
    } else {
	campaign=Campaigns=malloc(sizeof(Campaign));
    }
    NumCampaigns++;

    memset(campaign,0,sizeof(Campaign));
    campaign->Ident=ident;
    campaign->Players=1;
    tail=&campaign->Chapters;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    campaign->Name=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if ( gh_eq_p(value,gh_symbol2scm("players")) ) {
	    campaign->Players=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("campaign")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    //
	    //	Parse the list
	    //
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		chapter=calloc(sizeof(CampaignChapter),1);
		chapter->Next=*tail;
		*tail=chapter;
		tail=&chapter->Next;

		if( gh_eq_p(value,gh_symbol2scm("show-picture")) ) {
		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    chapter->Type=ChapterShowPicture;
		    chapter->Name=gh_scm2newstr(value,NULL);
		} else if( gh_eq_p(value,gh_symbol2scm("play-level")) ) {
		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    chapter->Type=ChapterPlayLevel;
		    chapter->Name=gh_scm2newstr(value,NULL);
		} else {
		   // FIXME: this leaves a half initialized campaign
		   errl("Unsupported tag",value);
		}
	    }
	} else {
	   // FIXME: this leaves a half initialized campaign
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for campaigns.
*/
global void CampaignCclRegister(void)
{
    gh_new_procedureN("define-campaign",CclDefineCampaign);
}

/**
**	Save the campaign module.
*/
global void SaveCampaign(FILE* file)
{
    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: campaign $Id$\n\n");
    fprintf(file,";;; FIXME: Save not written\n\n");
}

/**
**	Clean up the campaign module.
*/
global void CleanCampaign(void)
{
    // FIXME: Can't clean campaign needed for continue.
    DebugLevel0Fn("FIXME: Cleaning campaign not written\n");
}

//@}
