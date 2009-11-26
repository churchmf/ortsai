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

Captain::Captain(General& theGeneral)
{
	*general = theGeneral;
	HEALTHY_VALUE = 30;

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
		if (Lieutenants[i]->NeedsAid())
		{
			return Lieutenants[i]->GetLocation();
		}
	}
	return vec2(-1,-1);
}

void Captain::Loop(Vector<Lieutenant*> theLieutenants)
{
	SetLieutenants(theLieutenants);

	for(size_t i(0); i<Lieutenants.size(); ++i)
	{
		Lieutenant* lieutenant(Lieutenants[i]);
		//if (squad healthy and not engaged):
		if(!lieutenant->IsEngaged() && lieutenant->GetHealth() >= HEALTHY_VALUE)
		{
			//Sets the current lieutenants aidRequest to false since it is safe and healthy
			if (lieutenant->NeedsAid())
				lieutenant->SetAid(false);
			//check for aid requests
			vec2 aidLocation = CheckAid();
			if (true) //TODO
			{
				//choose safe deployment location towards endangered squad
				//lieutenant->MoveTo(aidLocation)
			}
			else
			{
				//choose safe deployment location towards nearest enemy location
			}
		}
		else
		{
			//if winning
			if(true) //TODO
				continue;
			else
			{
				//fallback and request for aid
				lieutenant->SetAid(true);
				vec2 retreatLocation = general->GetFallBackLocation(lieutenant->GetLocation());
				//lieutenant->MoveTo(retreatLocation)
			}
		}
	}
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
