/*
 * Lieutenant.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "Lieutenant.H"
#include "Application.H"
#include "Helper.H"

#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"

Lieutenant::Lieutenant(){
private Vector<Unit> marines, tanks;
private sint4 health;
const sint4 marine = 1;
const sint4 tank = 2;

}

Lieutenant::~Lieutenant() {

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

bool Lieutenant::RelieveUnit()
{
	return false;
}

sint4 Lieutenant::GetHealth()
{
	health = 0;
	for (int i = 0; i < marines.size(); ++i)
	{
		health += marines[i].GetHitpoints();
	}
	for (int j = 0; i < tanks.size(); ++j)
	{
		health += tanks[j].GetHitpoints();
	}
	return health;
}

bool Lieutenant::IsEngaged()
{
	return false;
}

vec2 Lieutenant::GetLocation()
{
	vec2 sumLocation = new vec2(0,0);
	for (int i = 0; i < marines.size(); ++i)
	{
		sumLocation += marines[i].GetPosition();
	}
	for (int j = 0; i < tanks.size(); ++j)
	{
		sumLocation += tanks[j].GetPosition();
	}

	location = sumLocation / (marines.size() + tanks.size());
	return location;
}

