//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name smokeparticle.cpp - The smoke particle. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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

#include <sstream>
#include <iomanip>

#include "stratagus.h"
#include "particle.h"
#include "video.h"




CSmokeParticle::CSmokeParticle(CPosition position, Animation *smoke) :
	CParticle(position), puff(smoke)
{
}

CSmokeParticle::~CSmokeParticle()
{
	delete puff;
}

void CSmokeParticle::draw()
{
	CPosition screenPos = ParticleManager.getScreenPos(pos);
	puff->draw(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
}

void CSmokeParticle::update(int ticks)
{
	puff->update(ticks);
	if (puff->isFinished()) {
		destroy();
		return;
	}

	// smoke rises
	const int smokeRisePerSecond = 22;
	pos.y -= ticks / 1000.f * smokeRisePerSecond;
}

//@}
