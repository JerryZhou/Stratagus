//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name minimap.h - The minimap headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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
//      $Id$

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class Minimap
{
public:
	static void UpdateXY(int tx, int ty);
	static void UpdateSeenXY(int tx, int ty) {}
	static void UpdateTerrain(void);
	static void Update(void);
	static void Create(void);
#ifdef USE_OPENGL
	static void Reload(void);
#endif
	static void Destroy(void);
	static void Draw(int vx, int vy);
	static void DrawCursor(int vx, int vy);
	static void AddEvent(int x, int y);

	static int Screen2MapX(int x);
	static int Screen2MapY(int y);

	static int X;
	static int Y;
	static int WithTerrain;
	static int Friendly;
	static int ShowSelected;
};

//@}

#endif // !__MINIMAP_H__
