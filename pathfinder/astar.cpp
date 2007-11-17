//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name astar.cpp - The a* path finder routines. */
//
//      (c) Copyright 1999-2006 by Lutz Sammer,Fabrice Rossi, Russell Smith,
//                                  Francois Beerten.
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

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

struct Node {
	char Direction;     /// Direction for trace back
	char InGoal;        /// is this point in the goal
	int CostFromStart;  /// Real costs to reach this point
};

struct Open {
	int X;     /// X coordinate
	int Y;     /// Y coordinate
	int O;     /// Offset into matrix
	int Costs; /// complete costs to goal
};

/// heuristic cost fonction for a star
#define AStarCosts(sx,sy,ex,ey) (abs(sx-ex)+abs(sy-ey))
// Other heuristic functions
// #define AStarCosts(sx,sy,ex,ey) 0
// #define AStarCosts(sx,sy,ex,ey) isqrt((abs(sx-ex)*abs(sx-ex))+(abs(sy-ey)*abs(sy-ey)))
// #define AStarCosts(sx,sy,ex,ey) max(abs(sx-ex),abs(sy-ey))
/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

//  Convert heading into direction.
//                      //  N NE  E SE  S SW  W NW
const int Heading2X[9] = {  0,+1,+1,+1, 0,-1,-1,-1, 0 };
const int Heading2Y[9] = { -1,-1, 0,+1,+1,+1, 0,-1, 0 };
const int XY2Heading[3][3] = { {7,6,5},{0,0,4},{1,2,3}};
/// cost matrix
static Node *AStarMatrix;
/// a list of close nodes, helps to speed up the matrix cleaning
static int *CloseSet;
static int Threshold;
static int OpenSetMaxSize;
static int AStarMatrixSize;
#define MAX_CLOSE_SET_RATIO 4
#define MAX_OPEN_SET_RATIO 8 // 10,16 to small

/// see pathfinder.h
int AStarFixedUnitCrossingCost;// = MaxMapWidth * MaxMapHeight;
int AStarMovingUnitCrossingCost = 5;
bool AStarKnowUnseenTerrain = false;
int AStarUnknownTerrainCost = 2;

static int AStarMapWidth;
static int AStarMapHeight;

/**
** The Open set is handled by a stored array
** the end of the array holds the item witht he smallest cost.
*/

/// The set of Open nodes
static Open *OpenSet;
/// The size of the open node set
static int OpenSetSize;

static unsigned char *VisionTable[3];
static int *VisionLookup;

static int (STDCALL *CostMoveToCallback)(int x, int y, void *data);
static int *CostMoveToCache;
static const int CacheNotSet = -5;

static void InitVisionTable(void)
{
	int *visionlist;
	int maxsize;
	int sizex;
	int sizey;
	int maxsearchsize;
	int i;
	int VisionTablePosition;
	int marker;
	int direction;
	int right;
	int up;
	int repeat;

	// Initialize Visiontable to large size, can't be more entries than tiles.
	VisionTable[0] = new unsigned char[AStarMapWidth * AStarMapWidth];
	VisionTable[1] = new unsigned char[AStarMapWidth * AStarMapWidth];
	VisionTable[2] = new unsigned char[AStarMapWidth * AStarMapWidth];

	VisionLookup = new int[AStarMapWidth + 2];

	visionlist = new int[AStarMapWidth * AStarMapWidth];
	//*2 as diagonal distance is longer

	maxsize = AStarMapWidth;
	maxsearchsize = AStarMapWidth;
	// Fill in table of map size
	for (sizex = 0; sizex < maxsize; ++sizex) {
		for (sizey = 0; sizey < maxsize; ++sizey) {
			visionlist[sizey * maxsize + sizex] = isqrt(sizex * sizex + sizey * sizey);
		}
	}

	VisionLookup[0] = 0;
	i = 1;
	VisionTablePosition = 0;
	while (i < maxsearchsize) {
		// Set Lookup Table
		VisionLookup[i] = VisionTablePosition;
		// Put in Null Marker
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 0;
		++VisionTablePosition;


		// find i in left column
		marker = maxsize * i;
		direction = 0;
		right = 0;
		up = 0;

		// If not on top row, continue
		do {
			repeat = 0;
			do {
				// search for repeating
				// Test Right
				if ((repeat == 0 || direction == 1) && visionlist[marker + 1] == i) {
					right = 1;
					up = 0;
					++repeat;
					direction = 1;
					++marker;
				} else if ((repeat == 0 || direction == 2) && visionlist[marker - maxsize] == i) {
					up = 1;
					right = 0;
					++repeat;
					direction = 2;
					marker = marker - maxsize;
				} else if ((repeat == 0 || direction == 3) && visionlist[marker + 1 - maxsize] == i &&
						visionlist[marker - maxsize] != i && visionlist[marker + 1] != i) {
					up = 1;
					right = 1;
					++repeat;
					direction = 3;
					marker = marker + 1 - maxsize;
				} else {
					direction = 0;
					break;
				}

			   // search right
			   // search up - store as down.
			   // search diagonal
			}  while (direction && marker > (maxsize * 2));
			if (right || up) {
				VisionTable[0][VisionTablePosition] = repeat;
				VisionTable[1][VisionTablePosition] = right;
				VisionTable[2][VisionTablePosition] = up;
				++VisionTablePosition;
			}
		} while (marker > (maxsize * 2));
		++i;
	}

	delete[] visionlist;
}

/**
** Init A* data structures
*/
void InitAStar(int mapWidth, int mapHeight, int (STDCALL *costMoveTo)(int x, int y, void *data))
{
	if (!AStarMatrix) {
		AStarMapWidth = mapWidth;
		AStarMapHeight = mapHeight;
		CostMoveToCallback = costMoveTo;

		AStarMatrixSize = sizeof(Node) * AStarMapWidth * AStarMapHeight;
		AStarMatrix = new Node[AStarMapWidth * AStarMapHeight];
		memset(AStarMatrix, 0, AStarMatrixSize);
		Threshold = AStarMapWidth * AStarMapHeight / MAX_CLOSE_SET_RATIO;
		CloseSet = new int[Threshold];
		OpenSetMaxSize = AStarMapWidth * AStarMapHeight / MAX_OPEN_SET_RATIO;
		OpenSet = new Open[OpenSetMaxSize];
		CostMoveToCache = new int[AStarMapWidth * AStarMapHeight];

		InitVisionTable();
	}
}

/**
** Free A* data structure
*/
void FreeAStar(void)
{
	if (AStarMatrix) {
		delete[] AStarMatrix;
		AStarMatrix = NULL;
		delete[] CloseSet;
		delete[] OpenSet;
	}
}

/**
** Prepare path finder.
*/
static void AStarPrepare(void)
{
	memset(AStarMatrix, 0, AStarMatrixSize);
}

/**
** Clean up the AStarMatrix
*/
static void AStarCleanUp(int num_in_close)
{
	if (num_in_close >= Threshold) {
		AStarPrepare();
	} else {
		for (int i = 0; i < num_in_close; ++i) {
			AStarMatrix[CloseSet[i]].CostFromStart = 0;
			AStarMatrix[CloseSet[i]].InGoal = 0;
		}
	}
}

/**
** Find the best node in the current open node set
** Returns the position of this node in the open node set 
*/
#define AStarFindMinimum() (OpenSetSize - 1)


/**
** Remove the minimum from the open node set
*/
static void AStarRemoveMinimum(int pos)
{
	Assert(pos == OpenSetSize - 1);

	OpenSetSize--;
}

/**
** Add a new node to the open set (and update the heap structure)
** Returns Pathfinder failed
*/
static int AStarAddNode(int x, int y, int o, int costs)
{
	int bigi, smalli;
	int midcost;
	int midi;
	
	if (OpenSetSize + 1 >= OpenSetMaxSize) {
		fprintf(stderr, "A* internal error: raise Open Set Max Size "
				"(current value %d)\n", OpenSetMaxSize);
		return PF_FAILED;
	}

	
	// find where we should insert this node.
	bigi = 0;
	smalli = OpenSetSize;

	// binary search where to insert the new node
	while (bigi < smalli) {
		midi = (smalli + bigi) >> 1;
		midcost = OpenSet[midi].Costs;
		if (costs > midcost) {
			smalli = midi;
		} else if (costs < midcost) {
			if (bigi == midi) {
				bigi++;
			} else {
				bigi = midi;
			}
		} else {
			bigi = midi;
			smalli = midi;
		}
	}

	if (OpenSetSize > bigi) { 
		// free a the slot for our node
		memmove(&OpenSet[bigi+1], &OpenSet[bigi], (OpenSetSize - bigi) * sizeof(Open));
	}

	// fill our new node
	OpenSet[bigi].X = x;
	OpenSet[bigi].Y = y;
	OpenSet[bigi].O = o;
	OpenSet[bigi].Costs = costs;
	++OpenSetSize;

	return 0;
}

/**
** Change the cost associated to an open node. 
** Can be further optimised knowing that the new cost MUST BE LOWER
** than the old one.
*/
static void AStarReplaceNode(int pos, int costs)
{
	Open node;

	// Remove the outdated node
	node = OpenSet[pos];
	OpenSetSize--;
	memmove(&OpenSet[pos], &OpenSet[pos+1], sizeof(Open) * (OpenSetSize-pos));

	// Re-add the node with the new cost
	AStarAddNode(node.X, node.Y, node.O, node.Costs);
}


/**
** Check if a node is already in the open set.
** Return -1 if not found and the position of the node in the table if found.
*/
static int AStarFindNode(int eo)
{
	for (int i = 0; i < OpenSetSize; ++i) {
		if (OpenSet[i].O == eo) {
			return i;
		}
	}
	return -1;
}

/**
**  Compute the cost of crossing tile (dx,dy)
**
**  @param x     X tile where to move.
**  @param y     Y tile where to move.
**  @param data  user data.
**
**  @return      -1 -> impossible to cross.
**                0 -> no induced cost, except move
**               >0 -> costly tile
*/
static int CostMoveTo(int x, int y, void *data)
{
	int *c = &CostMoveToCache[y * AStarMapWidth + x];
	if (*c != CacheNotSet) {
		return *c;
	}
	return (*c = CostMoveToCallback(x, y, data));
}

/**
**  MarkAStarGoal
*/
static int AStarMarkGoal(int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange, int maxrange, int *num_in_close, void *data)
{
	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int x;
	int y;
	bool goal_reachable;
	int quad;
	int eo;
	int filler;
	int range;
	int z1, z2;
	bool doz1, doz2;

	goal_reachable = false;

	if (minrange == 0 && maxrange == 0 && gw == 0 && gh == 0) {
		if (gx + tilesizex >= AStarMapWidth ||
				gy + tilesizey >= AStarMapHeight) {
			return 0;
		}
		if (CostMoveTo(gx, gy, data) >= 0) {
			AStarMatrix[gx + gy * AStarMapWidth].InGoal = 1;
			return 1;
		} else {
			return 0;
		}
	}

	if (minrange == 0) {
		int sx = std::max(gx, 0);
		int ex = std::min(gx + gw, AStarMapWidth - tilesizex);
		for (x = sx; x < ex; ++x) {
			int sy = std::max(gy, 0);
			int ey = std::min(gy + gh, AStarMapHeight - tilesizey);
			for (y = sy; y < ey; ++y) {
				if (CostMoveTo(x, y, data) >= 0) {
					AStarMatrix[y * AStarMapWidth + x].InGoal = 1;
					goal_reachable = true;
				}
				if (*num_in_close < Threshold) {
					CloseSet[(*num_in_close)++] = y * AStarMapWidth + x;
				}
			}
		}
	}

	if (gw) {
		gw--;
	}
	if (gh) {
		gh--;
	}

	int sx = std::max(gx, 0);
	int ex = std::min(gx + gw, AStarMapWidth - tilesizex);
	int sy = std::max(gy, 0);
	int ey = std::min(gy + gh, AStarMapHeight - tilesizey);

	// Mark top, bottom, left, right
	for (range = minrange; range <= maxrange; ++range) {
		z1 = gy - range;
		z2 = gy + range + gh;
		doz1 = z1 >= 0 && z1 + tilesizex - 1 < AStarMapHeight;
		doz2 = z2 >= 0 && z2 + tilesizey - 1 < AStarMapHeight;
		if (doz1 || doz2) {
			// Mark top and bottom of goal
			for (x = sx; x <= ex; ++x) {
				if (doz1 && CostMoveTo(x, z1, data) >= 0) {
					AStarMatrix[z1 * AStarMapWidth + x].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = z1 * AStarMapWidth + x;
					}
					goal_reachable = true;
				}
				if (doz2 && CostMoveTo(x, z2, data) >= 0) {
					AStarMatrix[z2 * AStarMapWidth + x].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = z2 * AStarMapWidth + x;
					}
					goal_reachable = true;
				}
			}
		}
		z1 = gx - range;
		z2 = gx + gw + range;
		doz1 = z1 >= 0 && z1 + tilesizex - 1 < AStarMapWidth;
		doz2 = z2 >= 0 && z2 + tilesizex - 1 < AStarMapWidth;
		if (doz1 || doz2) {
			// Mark left and right of goal
			for (y = sy; y <= ey; ++y) {
				if (doz1 && CostMoveTo(z1, y, data) >= 0) {
					AStarMatrix[y * AStarMapWidth + z1].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = y * AStarMapWidth + z1;
					}
					goal_reachable = true;
				}
				if (doz2 && CostMoveTo(z2, y, data) >= 0) {
					AStarMatrix[y * AStarMapWidth + z2].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = y * AStarMapWidth + z2;
					}
					goal_reachable = true;
				}
			}
		}
	} // Go Through the Ranges

	// Mark Goal Border in Matrix

	// Mark Edges of goal

	steps = VisionLookup[minrange];

	while (VisionTable[0][steps] <= maxrange) {
		// 0 - Top right Quadrant
		cx[0] = gx + gw;
		cy[0] = gy - VisionTable[0][steps];
		// 1 - Top left Quadrant
		cx[1] = gx;
		cy[1] = gy - VisionTable[0][steps];
		// 2 - Bottom Left Quadrant
		cx[2] = gx;
		cy[2] = gy + VisionTable[0][steps]+gh;
		// 3 - Bottom Right Quadrant
		cx[3] = gx + gw;
		cy[3] = gy + VisionTable[0][steps]+gh;

		++steps;  // Move past blank marker
		while (VisionTable[1][steps] != 0 || VisionTable[2][steps] != 0) {
			// Loop through for repeat cycle
			cycle = 0;
			while (cycle++ < VisionTable[0][steps]) {
				// If we travelled on an angle, mark down as well.
				if (VisionTable[1][steps] == VisionTable[2][steps]) {
					// do down
					for (quad = 0; quad < 4; ++quad) {
						if (quad < 2) {
							filler = 1;
						} else {
							filler = -1;
						}
						if (cx[quad] >= 0 && cx[quad] + tilesizex - 1 < AStarMapWidth &&
								cy[quad] + filler >= 0 && cy[quad] + filler + tilesizey - 1 < AStarMapHeight &&
								CostMoveTo(cx[quad], cy[quad] + filler, data) >= 0) {
							eo = (cy[quad] + filler) * AStarMapWidth + cx[quad];
							AStarMatrix[eo].InGoal = 1;
							if (*num_in_close < Threshold) {
								CloseSet[(*num_in_close)++] = eo;
							}
							goal_reachable = true;
						}
					}
				}

				cx[0] += VisionTable[1][steps];
				cy[0] += VisionTable[2][steps];
				cx[1] -= VisionTable[1][steps];
				cy[1] += VisionTable[2][steps];
				cx[2] -= VisionTable[1][steps];
				cy[2] -= VisionTable[2][steps];
				cx[3] += VisionTable[1][steps];
				cy[3] -= VisionTable[2][steps];

				// Mark Actually Goal curve change
				for (quad = 0; quad < 4; ++quad) {
					if (cx[quad] >= 0 && cx[quad] + tilesizex - 1 < AStarMapWidth &&
							cy[quad] >= 0 && cy[quad] + tilesizey - 1 < AStarMapHeight &&
							CostMoveTo(cx[quad], cy[quad], data) >= 0) {
						eo = cy[quad] * AStarMapWidth + cx[quad];
						AStarMatrix[eo].InGoal = 1;
						if (*num_in_close < Threshold) {
							CloseSet[(*num_in_close)++] = eo;
						}
						goal_reachable = true;
					}
				}
			}
			++steps;
		}
	}
	return goal_reachable;
}

/**
** Find path.
*/
int AStarFindPath(int sx, int sy, int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange, int maxrange, char *path, int pathlen, void *data)
{
	int i;
	int j;
	int o;
	int ex;
	int ey;
	int eo;
	int x;
	int y;
	int px;
	int py;
	int shortest;
	int counter;
	int new_cost;
	int cost_to_goal;
	int path_length;
	int num_in_close;

	if (maxrange == 0 && abs(gx - sx) <= 1 && abs(gy - sy) <= 1) {
		// Simplest case, move to adj cell
		if (gx == sx && gy == sy) {
			return PF_REACHED;
		}

		if (path) {
			path[0] = XY2Heading[gx - sx + 1][gy - sy + 1];
		}
		return 1;
	}

	for (i = 0; i < AStarMapWidth * AStarMapHeight; ++i) {
		CostMoveToCache[i] = CacheNotSet;
	}

	OpenSetSize = 0;
	num_in_close = 0;
	x = sx;
	y = sy;

	// if goal is not directory reachable, punch out
	if (!AStarMarkGoal(gx, gy, gw, gh, tilesizex, tilesizey,
			minrange, maxrange, &num_in_close, data)) {
		AStarCleanUp(num_in_close);
		return PF_UNREACHABLE;
	}

	eo = y * AStarMapWidth + x;
	// it is quite important to start from 1 rather than 0, because we use
	// 0 as a way to represent nodes that we have not visited yet.
	AStarMatrix[eo].CostFromStart = 1;
	// 8 to say we are came from nowhere.
	AStarMatrix[eo].Direction = 8;

	// place start point in open, it that failed, try another pathfinder
	if (AStarAddNode(x, y, eo, 1 + AStarCosts(x, y, gx, gy)) == PF_FAILED) {
		AStarCleanUp(num_in_close);
		return PF_FAILED;
	}
	if (num_in_close < Threshold) {
		CloseSet[num_in_close++] = OpenSet[0].O;
	}
	if (AStarMatrix[eo].InGoal) {
		AStarCleanUp(num_in_close);
		return PF_REACHED;
	}

	counter = AStarMapWidth * AStarMapHeight;

	while (1) {
		//
		// Find the best node of from the open set
		//
		shortest = AStarFindMinimum();
		x = OpenSet[shortest].X;
		y = OpenSet[shortest].Y;
		o = OpenSet[shortest].O;
		cost_to_goal = OpenSet[shortest].Costs - AStarMatrix[o].CostFromStart;

		AStarRemoveMinimum(shortest);

		//
		// If we have reached the goal, then exit.
		if (AStarMatrix[o].InGoal == 1) {
			ex = x;
			ey = y;
			break;
		}

		//
		// If we have looked too long, then exit.
		//
		if (!counter--) {
			//
			// Select a "good" point from the open set.
			// Nearest point to goal.
			AstarDebugPrint("way too long\n");
			AStarCleanUp(num_in_close);
			return PF_FAILED;
		}

		//
		// Generate successors of this node.

		// Node that this node was generated from.
		px = x - Heading2X[(int)AStarMatrix[x + AStarMapWidth * y].Direction];
		py = y - Heading2Y[(int)AStarMatrix[x + AStarMapWidth * y].Direction];

		for (i = 0; i < 8; ++i) {
			ex = x + Heading2X[i];
			ey = y + Heading2Y[i];

			// Don't check the tile we came from, it's not going to be better
			// Should reduce load on A*

			if (ex == px && ey == py) {
				continue;
			}
			//
			// Outside the map or can't be entered.
			//
			if (ex < 0 || ex + tilesizex - 1 >= AStarMapWidth ||
					ey < 0 || ey + tilesizey - 1 >= AStarMapHeight) {
				continue;
			}

			// if the point is "move to"-able and
			// if we have not reached this point before,
			// or if we have a better path to it, we add it to open set
			new_cost = CostMoveTo(ex, ey, data);
			if (new_cost == -1) {
				// uncrossable tile
				continue;
			}

			// Add a cost for walking to make paths more realistic for the user.
			new_cost += abs(Heading2X[i]) + abs(Heading2Y[i]) + 1;
			eo = ey * AStarMapWidth + ex;
			new_cost += AStarMatrix[o].CostFromStart;
			if (AStarMatrix[eo].CostFromStart == 0) {
				// we are sure the current node has not been already visited
				AStarMatrix[eo].CostFromStart = new_cost;
				AStarMatrix[eo].Direction = i;
				if (AStarAddNode(ex, ey, eo, AStarMatrix[eo].CostFromStart + AStarCosts(ex, ey, gx, gy)) == PF_FAILED) {
					AStarCleanUp(num_in_close);
					return PF_FAILED;
				}
				// we add the point to the close set
				if (num_in_close < Threshold) {
					CloseSet[num_in_close++] = eo;
				}
			} else if (new_cost < AStarMatrix[eo].CostFromStart) {
				// Already visited node, but we have here a better path
				// I know, it's redundant (but simpler like this)
				AStarMatrix[eo].CostFromStart = new_cost;
				AStarMatrix[eo].Direction = i;
				// this point might be already in the OpenSet
				j = AStarFindNode(eo);
				if (j == -1) {
					if (AStarAddNode(ex, ey, eo,
							AStarMatrix[eo].CostFromStart + AStarCosts(ex, ey, gx, gy)) == PF_FAILED) {
						AStarCleanUp(num_in_close);
						return PF_FAILED;
					}
				} else {
					AStarReplaceNode(j, AStarMatrix[eo].CostFromStart + AStarCosts(ex, ey, gx, gy));
				}
				// we don't have to add this point to the close set
			}
		}
		if (OpenSetSize <= 0) { // no new nodes generated
			AStarCleanUp(num_in_close);
			return PF_UNREACHABLE;
		}
	}

	// now we need to backtrack
	path_length = 0;
	x = sx;
	y = sy;
	gx = ex;
	gy = ey;
	i = 0;
	while (ex != x || ey != y) {
		eo = ey * AStarMapWidth + ex;
		i = AStarMatrix[eo].Direction;
		ex -= Heading2X[i];
		ey -= Heading2Y[i];
		path_length++;
	}

	// gy = Path length to cache
	// gx = Current place in path
	ex = gx;
	ey = gy;
	gy = path_length;
	gx = path_length;
	if (gy >= pathlen) {
		gy = pathlen - 1;
	}

	// Now we have the length, calculate the cached path.
	while ((ex != x || ey != y) && path != NULL) {
		eo = ey * AStarMapWidth + ex;
		i = AStarMatrix[eo].Direction;
		ex -= Heading2X[i];
		ey -= Heading2Y[i];
		--gx;
		if (gx < gy) {
			path[gy - gx - 1] = i;
		}
	}

	// let's clean up the matrix now
	AStarCleanUp(num_in_close);
	if ((AStarMapWidth * AStarMapHeight) - counter > 500) {
		AstarDebugPrint("Visited %d tiles, length %d tiles\n" _C_
			(AStarMapWidth * AStarMapHeight) - counter _C_ path_length);
	}
	return path_length;
}

// AStarFixedUnitCrossingCost
void SetAStarFixedUnitCrossingCost(int cost) {
	if (cost <= 3) {
		fprintf(stderr, "AStarFixedUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarFixedUnitCrossingCost() {
	return AStarFixedUnitCrossingCost;
}

// AStarMovingUnitCrossingCost
void SetAStarMovingUnitCrossingCost(int cost) {
	if (cost <= 3) {
		fprintf(stderr, "AStarMovingUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarMovingUnitCrossingCost() {
	return AStarMovingUnitCrossingCost;
}

// AStarUnknownTerrainCost
void SetAStarUnknownTerrainCost(int cost) {
	if (cost < 0) {
		fprintf(stderr, "AStarUnknownTerrainCost must be non-negative\n");
		return;
	}
	AStarUnknownTerrainCost = cost;
}
int GetAStarUnknownTerrainCost() {
	return AStarUnknownTerrainCost;
}


//@}
