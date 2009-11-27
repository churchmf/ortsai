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
	general = &theGeneral;
	HEALTHY_VALUE = 30;

}

Captain::~Captain()
{

}

void Captain::SetLieutenants(Vector<Lieutenant*> theLieutenants)
{
	Lieutenants = theLieutenants;
}

bool Captain::existsAidRequest()
{
	for(size_t i(0); i<Lieutenants.size(); ++i)
	{
		if (Lieutenants[i]->NeedsAid())
		{
			return true;
		}
	}
	return false;
}

vec2 Captain::ProvideAid()
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
		if(lieutenant->GetHealth() >= HEALTHY_VALUE && !lieutenant->IsEngaged())
		{
			//Sets the current lieutenants aidRequest to false since it is safe and healthy
			if (lieutenant->NeedsAid())
				lieutenant->SetAid(false);
			//check for aid requests
			if (existsAidRequest())
			{
				//choose safe deployment location towards endangered squad
				vec2 aidLocation = ProvideAid();
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
			if(!general->isOutNumbered(lieutenant->GetLocation()))
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
}
