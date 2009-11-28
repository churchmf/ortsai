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

//////////////////////////////////////////////////////////////
////////////    CONSTANTS AND GAME VARIABLES      ///////////
/////////////////////////////////////////////////////////////
//constant values for marine and tanks used with unit.GetType()
const sint4 MARINE = 1;
const sint4 TANK = 2;

//maximum number of marines and tanks per lieutenant squad
const sint4 MAX_MARINES = 10;
const sint4 MAX_TANKS = 4;

//Value to represent if a squad is healthy or not
const sint4 HEALTHY_VALUE = 30;
//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////


Captain::Captain(General& theGeneral)
{
	general = &theGeneral;
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
			return Lieutenants[i]->GetCurrentPosition();
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
			if(!general->isOutNumbered(lieutenant->GetCurrentPosition()))
				continue;
			else
			{
				//fallback and request for aid
				lieutenant->SetAid(true);
				vec2 retreatLocation = general->GetFallBackLocation(lieutenant->GetCurrentPosition());
				//lieutenant->MoveTo(retreatLocation)
			}
		}
	}
}
