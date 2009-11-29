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
	delete general;
	Lieutenants.clear();
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

vec2 Captain::GetAidRequestLocation()
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

//currently static, but easy to modify
void Captain::Deploy()
{
	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		switch(i)
		{
		case 0:
			Lieutenants[i]->SetGoal(vec2(130, 100));
			Lieutenants[i]->DoFormation(vec2(1, 0));
			break;
		case 1:
			Lieutenants[i]->SetGoal(vec2(80, 220));
			Lieutenants[i]->DoFormation(vec2(1,0));
			break;
		case 2:
			Lieutenants[i]->SetGoal(vec2(80, 390));
			Lieutenants[i]->DoFormation(vec2(1,0));
			break;
		case 3:
			Lieutenants[i]->SetGoal(vec2(80, 560));
			Lieutenants[i]->DoFormation(vec2(1,0));
			break;
		case 4:
			Lieutenants[i]->SetGoal(vec2(130, 670));
			Lieutenants[i]->DoFormation(vec2(1,0));
			break;
		}
	}
}

//THOUGHTS: I think we need to implement a queue of task/"orders" system with the lieut so he does not get confused when given multiple orders, he will execute them in order
void Captain::Loop()
{
	for(size_t i(0); i<Lieutenants.size(); ++i)
	{
		Lieutenant* lieutenant(Lieutenants[i]);
		//std::cout << lieutenant->GetHealth() << std::endl;
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
				vec2 aidLocation = GetAidRequestLocation();
				//std::cout << aidLocation.x << "," << aidLocation.y << std::endl;
				//lieutenant->MoveTo(aidLocation);
			}
			else
			{
				//choose safe deployment location towards nearest enemy location
				vec2 enemy = general->GetClosestTarget(lieutenant->GetCurrentPosition());
				//lieutenant->MoveTo(enemy);
			}
		}
		else
		{
			//if winning
			//std::cout << general->IsOutNumbered(lieutenant->GetCurrentPosition()) << std::endl;
			if(!general->IsOutNumbered(lieutenant->GetCurrentPosition()))
			{

			}
			else
			{
				//fallback and request for aid
				lieutenant->SetAid(true);
				vec2 retreatLocation = general->GetFallBackLocation(lieutenant->GetCurrentPosition());
				//std::cout << retreatLocation.x << "," << retreatLocation.y << std::endl;
				//lieutenant->MoveTo(retreatLocation);
			}
		}
	}
}
