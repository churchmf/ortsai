/*
 * Captain.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "Captain.H"

#include "Application.H"
#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"

Captain::Captain()
{

}

Captain::~Captain()
{

}

void Captain::SetLieutenants(Vector<Lieutenant*> theLieutenants)
{
	Lieutenants = theLieutenants;
}

vec2 Captain::CheckAid()
{
	for(size_t i(0); i<Lieutenants.size(); ++i)
	{
		if (Lieutenants[i]->RequestsAid())
		{
			return Lieutenants[i]->GetLocation();
		}
	}
	return vec2(-1,-1);
}

//TODO: we might need to rework strategy now that tanks don't siege
Lieutenant* Captain::GetFurthestLieut()
{

}

void Captain::Loop(Vector<Lieutenant*> theLieutenants)
{
	SetLieutenants(theLieutenants);
	/*
	choose Lieutenant furthest from Enemy location
	if (squad healthy and not engaged):
	  check for nearest aid request
	  if (aid request):
	    choose safe deployment location towards endangered squad
	  else:
	    choose safe deployment location towards nearest enemy location
	else:
	  if (winning):
	    continue fighting
	  else:
	    fallback
	    request aid
	*/
}
