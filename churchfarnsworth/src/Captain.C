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

//The number of frames before executing the main Captain loop
const sint4 DEPLOY_TIME = 120;
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

void Captain::RemoveLieutenant(size_t index)
{
	if (index < Lieutenants.size())
		Lieutenants.erase(Lieutenants.begin()+index);
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
	//determines which side of map we are spawned
	vec2 position = Lieutenants[0]->GetCurrentPosition();
	vec2 adjustment;//the side the lieutenant was spawned
	vec2 facing;	//direction deployment should face
	if (position.x < (general->width)/2)
	{
		adjustment = vec2(0,0);
		facing = vec2(1.0,0.0);
		std::cout << "LEFT SIDE" << std::endl;

	}
	else
	{
		adjustment = vec2((sint4)general->width,0);
		facing = vec2(-1.0,0.0);
		std::cout << "RIGHT SIDE" << std::endl;
	}

	//determine each lieutenants position relative to the side it was spawned
	vec2 lieut1 = vec2(0,0);
	lieut1.x = abs(adjustment.x - 130);
	lieut1.y = abs(adjustment.y - 100);
	vec2 lieut2 = vec2(0,0);
	lieut2.x = abs(adjustment.x - 80);
	lieut2.y = abs(adjustment.y - 220);
	vec2 lieut3 = vec2(0,0);
	lieut3.x = abs(adjustment.x - 80);
	lieut3.y = abs(adjustment.y - 390);
	vec2 lieut4 = vec2(0,0);
	lieut4.x = abs(adjustment.x - 80);
	lieut4.y = abs(adjustment.y - 560);
	vec2 lieut5 = vec2(0,0);
	lieut5.x = abs(adjustment.x - 130);
	lieut5.y = abs(adjustment.y - 670);

	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		switch(i)
		{
		case 0:
			Lieutenants[i]->SetGoal(lieut1);
			Lieutenants[i]->DoFormation(facing);
			break;
		case 1:
			Lieutenants[i]->SetGoal(lieut2);
			Lieutenants[i]->DoFormation(facing);
			break;
		case 2:
			Lieutenants[i]->SetGoal(lieut3);
			Lieutenants[i]->DoFormation(facing);
			break;
		case 3:
			Lieutenants[i]->SetGoal(lieut4);
			Lieutenants[i]->DoFormation(facing);
			break;
		case 4:
			Lieutenants[i]->SetGoal(lieut5);
			Lieutenants[i]->DoFormation(facing);
			break;
		}
	}
}

void Captain::DistributeUnits(Vector<Unit> units)
{
	uint4 m = 0;
	uint4 t = 0;
	for(size_t i(0); i< units.size(); ++i)
	{
		Unit & unit(units[i]);

		if(unit.HasWeapon())
		{
			if(unit.GetType() == MARINE)
			{
				if (Lieutenants[m]->MarineSize() >= MAX_MARINES
				&& m < Lieutenants.size())
					m++;
				Lieutenants[m]->AssignUnit(unit);
			}
			if(unit.GetType() == TANK)
			{
				if (Lieutenants[t]->TankSize() >= MAX_TANKS
				&& t < Lieutenants.size())
					t++;
				Lieutenants[t]->AssignUnit(unit);
			}
		}
	}
}

void Captain::Loop(const sint4 frame)
{
	for(size_t i(0); i<Lieutenants.size(); ++i)
	{
		Lieutenant* lieutenant(Lieutenants[i]);

		//formation initalization time
		if (frame < DEPLOY_TIME)
		{
			lieutenant->CheckFormation();
			return;
		}

		//if the squad is not engaged
		if(!lieutenant->IsEngaged())
		{
			std::cout << "NOT ENGAGED" << std::endl;
			//if the squad is healthy
			if (lieutenant->IsHealthy())
			{
				std::cout << "HEALTHY" << std::endl;
				//if the squad is current not executing an order
				if (!lieutenant->HasOrder())
				{
					//check for aid requests
					if (existsAidRequest())
					{
						std::cout << "HELPING" << std::endl;
						//choose safe deployment location towards endangered squad
						vec2 aidLocation = GetAidRequestLocation();
						//find a safe waypoint towards aidLocation
						vec2 safeMove = general->FindSafeWaypoint(lieutenant->GetCurrentPosition(), aidLocation);
						//move towards aid call, if a safe path exists
						lieutenant->MoveTo(safeMove);
					}
					else
					{
						std::cout << "ATTACK" << std::endl;
						//choose safe deployment location towards nearest enemy location
						vec2 enemy = general->GetClosestTarget(lieutenant->GetCurrentPosition());
						//find a safe waypoint towards enemy
						vec2 safeMove = general->FindSafeWaypoint(lieutenant->GetCurrentPosition(), enemy);
						//move towards enemy if a safe path exists
						lieutenant->MoveTo(safeMove);
					}
				}
			}
			else
			{
				std::cout << "REFORMING" << std::endl;
				//if another unhealthy squad exists, merge with them
				Vector<Unit> transfers = lieutenant->TransferSquad();
				RemoveLieutenant(i);
				DistributeUnits(transfers);

			}
		}
		else	//Lieutenant is in combat
		{
			// Lieutenant is loosing
			if(general->IsOutNumbered(lieutenant->GetCurrentPosition()))
			{
				std::cout << "RETREATING" << std::endl;
				//retreat and request for aid
				lieutenant->SetAid(true);
				vec2 retreatLocation = general->GetFallBackLocation(lieutenant->GetCurrentPosition());
				std::cout << retreatLocation.x << "," << retreatLocation.y << std::endl;
				lieutenant->MoveTo(retreatLocation);
			}
		}
	}
}
