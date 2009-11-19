/*
 * Lieutenant.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "Lieutenant.H"
Vector<Unit> marines, tanks;
sint4 health;
vec2 location;
const sint4 marine = 1;
const sint4 tank = 2;

Lieutenant::Lieutenant()
{
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

bool Lieutenant::RelieveUnit(Unit unit)
{
	return false;
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
	return health;
}

bool Lieutenant::IsEngaged()
{
	return false;
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

	location = sumLocation / (marines.size() + tanks.size());
	return location;
}

//default, needs updating
void Lieutenant::DoFormation()
{
	location = GetLocation();
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		marine.MoveTo(location, marine.GetMaxSpeed());
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		const Unit & tank(tanks[j]);
		tank.MoveTo(location, tank.GetMaxSpeed());
	}
}

