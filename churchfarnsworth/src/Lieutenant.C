/*
 * Lieutenant.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "Lieutenant.H"
#include "Movement.H"
#include "Application.H"
#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"
#include "ST_ForceField.H"

//constant values for marine and tanks used with unit.GetType()
const sint4 marine = 1;
const sint4 tank = 2;

Lieutenant::Lieutenant()
{
	engaged = 0;
	requestsAid = 0;
	health = 0;
	location = vec2(0,0);
}

Lieutenant::~Lieutenant()
{

}

void Lieutenant::AssignUnit(Unit unit)
{
	if (unit.GetType() == marine)
	{
		marines.push_back(unit);
	}
	else if (unit.GetType() == tank)
	{
		tanks.push_back(unit);
	}
}

void Lieutenant::RelieveUnit(sint4 type, uint4 index)
{
	if (type == marine)
	{
		if (index < marines.size())
			marines.erase(marines.begin()+(index-1));
	}
	else if (type == tank)
	{
		if (index < tanks.size())
			tanks.erase(tanks.begin()+(index-1));
	}
}

sint4 Lieutenant::GetHealth()
{
	health = 0;
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		health += marine.GetHitpoints();
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		const Unit & tank(tanks[j]);
		health += tank.GetHitpoints();
	}
	sint4 percent = 100*(health / (marines.size()*80 + tanks.size()*150));
	return  percent;
}

bool Lieutenant::IsEngaged()
{
	return engaged;
}

void Lieutenant::UpdateEngaged()
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		if (marine.InCombat())
		{
			engaged = true;
			return;
		}
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		const Unit & tank(tanks[j]);
		if (tank.InCombat())
		{
			engaged = true;
			return;
		}
	}
}

bool Lieutenant::NeedsAid()
{
	return requestsAid;
}

void Lieutenant::SetAid(bool aid)
{
	requestsAid = aid;
}


vec2 Lieutenant::GetLocation()
{
	vec2 sumLocation = vec2(0,0);
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		sumLocation = sumLocation + marine.GetPosition();
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		const Unit & tank(tanks[j]);
		sumLocation = sumLocation + tank.GetPosition();
	}
	sint4 squadSize = marines.size() + tanks.size();
	location = vec2(sumLocation.x/squadSize, sumLocation.y/squadSize);
	return location;
}

//default, needs updating
//NOT OPTIMAL pattern
void Lieutenant::DoFormation(vec2 dir)
{
	location = GetLocation();
	vec2 initUnitPos = location;
	vec2 unitPos = initUnitPos;

	//TODO: make this dynamic to what direction you are pointing
	vec2 ltDirection = dir;

	vec2 rowDirection = vec2(-ltDirection.y, ltDirection.x);//direction of the rows

	//if new units added just change these variables
	sint4 unitDisplace = 0;//variable used to displace the rows evenly
	//NOTE: this variable is used do determine the distance of the row
	//		from the LT. so if more units get added, this can just be
	//		incremented for every for loop and each subsiquent row will
	//		be 'unitOffset' closer to the LT for the previous row.
	sint4 marineOffset = 24;//distance between each marine in the row
	sint4 tankOffset = 30;//distance between the tanks in the row
	sint4 unitOffset = 40;//space between rows
	sint4 frontLine = 60;//distance of frontline from LT

	sint4 displace = 1;



	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		//marine.MoveTo(location, marine.GetMaxSpeed());
		//set first unit
		if(i == 0)
		{
			initUnitPos = vec2((ltDirection.x * frontLine) + location.x, (ltDirection.y * frontLine) + location.y);
			mc->moveUnit(marine.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));

		}
		else if(i % 2 == 1 && i > 0)
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2((rowDirection.x * marineOffset * displace) + initUnitPos.x, (rowDirection.y * marineOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			mc->moveUnit(marine.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));

		}
		//set every second unit (ones on negative plane to LT) to negative
		else if(i % 2 == 0 && i > 0)
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2(((-rowDirection.x) * marineOffset * displace) + initUnitPos.x, ((-rowDirection.y) * marineOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			mc->moveUnit(marine.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));

			displace++;
		}
	}

	unitDisplace++;
	displace = 1;

	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		if(j == 0)
		{
			initUnitPos = vec2((ltDirection.x * frontLine) - (unitDisplace*unitOffset) + location.x,
							   (ltDirection.y * frontLine) - (unitDisplace*unitOffset) + location.y);
			mc->moveUnit(tank.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));
		}
		else if(j % 2 == 1 && j > 0)
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2((rowDirection.x * tankOffset * displace) + initUnitPos.x, (rowDirection.y * tankOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			mc->moveUnit(tank.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));
		}
		//set every second unit (ones on negative plane to LT) to negative
		else if(j % 2 == 0 && j > 0)
		{
			//set the next tank at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2(((-rowDirection.x) * tankOffset * displace) + initUnitPos.x, ((-rowDirection.y) * tankOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			mc->moveUnit(tank.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(unitPos.x, unitPos.y)));
			displace++;
		}
	}
}

//default, needs updating
void Lieutenant::MoveTo(vec2 target)
{
	location = target;
	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		mc->moveUnit(marine.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(location.x,location.y)));
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		mc->moveUnit(tank.GetGameObj(), Movement::TouchPoint(Movement::Vec2D(location.x,location.y)));;
	}
}

void Lieutenant::Loop(Movement::Context& MC)
{
	mc = &MC;
	/*
	 *  if (orders):
		   do order
		else:
		  if (engaged):
			for each unit in squad:
			  if expected death:
				pull back
		  else:
			target weakest enemy unit in range
			attack enemy unit
		update squad status
	 */

}

