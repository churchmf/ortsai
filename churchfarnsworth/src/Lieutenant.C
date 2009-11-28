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

//////////////////////////////////////////////////////////////
////////////    CONSTANTS AND GAME VARIABLES      ///////////
/////////////////////////////////////////////////////////////
//constant values for marine and tanks used with unit.GetType()
const sint4 MARINE = 1;
const sint4 TANK = 2;

//maximum number of marines and tanks per lieutenant squad
const sint4 MAX_MARINES = 10;
const sint4 MAX_TANKS = 4;

const sint4 MAX_RANGE = 112;

//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////

Lieutenant::Lieutenant()
{
	engaged = 0;
	requestsAid = 0;
	health = 0;
	location = vec2(0,0);
	INIT_FLAG = false;
}

Lieutenant::~Lieutenant()
{
}

void Lieutenant::AssignUnit(Unit unit)
{
	if (unit.GetType() == MARINE)
	{
		marines.push_back(unit);
	}
	else if (unit.GetType() == TANK)
	{
		tanks.push_back(unit);
	}
}

void Lieutenant::RelieveUnit(sint4 type, uint4 index)
{
	if (type == MARINE)
	{
		if (index < marines.size());
			//marines.erase(marines.begin()+(index-1));
	}
	else if (type == TANK)
	{
		if (index < tanks.size());
			//tanks.erase(tanks.begin()+(index-1));
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
	sint4 percent = 100*(health / (MAX_MARINES*80 + MAX_TANKS*150));
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

void Lieutenant::CasualtyCheck()
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		if (!marine.IsAlive())
		{
			RelieveUnit(MARINE, i);
		}
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		const Unit & tank(tanks[j]);
		if (!tank.IsAlive())
		{
			RelieveUnit(TANK, j);
		}
	}
}

//TODO: Change. DO NOT USE AVERAGE LOCATION
//RETURNING GOAL JUST FOR TESTING
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
	vec2 location = vec2(sumLocation.x/squadSize, sumLocation.y/squadSize);

	return goal;
}

void Lieutenant::SetGoal(vec2 g)
{
	goal = g;
}

vec2 Lieutenant::GetGoal()
{
	return goal;
}

//NOTE: VARIABLES MUST BE INITIALIZED BEFORE CALLING THIS FUNCTION
void Lieutenant::CheckFormation()
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		//if the unit fails to get to its location or it thinks it is at its location
		//check its location to its goal, if its not there move it to its goal
		if((*marines[i].GetTask()).getStatus() == Movement::Task::MOVE_FAILURE ||
		   (*marines[i].GetTask()).getStatus() == Movement::Task::SUCCESS &&
		   marines[i].IsAlive())
		{
			//std::cout << "MOVEMENT FAILED" << std::endl;

			if(!((int)marines[i].GetPosition().x == (int)marines[i].GetVector().x &&
			     (int)marines[i].GetPosition().y == (int)marines[i].GetVector().y))
			{
				(*marines[i].GetTask()).cancel();
				marines[i].SetTask(mc->moveUnit(marines[i].GetGameObj(), marines[i].GetGoal()));

				//std::cout << "MRN Pos: " << marines[i].GetPosition().x << ", " << marines[i].GetPosition().y << std::endl;
				//std::cout << "MRN Goal: " << marines[i].GetVector().x << ", " << marines[i].GetVector().y << std::endl;

				//std::cout << "NEW MOVESET" << std::endl;
			}
			//exit(0);
		}
	}

	for (size_t i(0); i < tanks.size(); ++i)
	{
		if((*tanks[i].GetTask()).getStatus() == Movement::Task::MOVE_FAILURE ||
		   (*tanks[i].GetTask()).getStatus() == Movement::Task::SUCCESS      &&
		   tanks[i].IsAlive())
		{
			if(!((int)tanks[i].GetPosition().x == (int)tanks[i].GetVector().x &&
				 (int)tanks[i].GetPosition().y == (int)tanks[i].GetVector().y))
			{
				//std::cout << "MOVEMENT FAILED" << std::endl;

				(*tanks[i].GetTask()).cancel();
				tanks[i].SetTask(mc->moveUnit(tanks[i].GetGameObj(), tanks[i].GetGoal()));

				//std::cout << "TNK Pos: " << tanks[i].GetPosition().x << ", " << tanks[i].GetPosition().y << std::endl;
				//std::cout << "TNK Goal: " << tanks[i].GetVector().x << ", " << tanks[i].GetVector().y << std::endl;

				//std::cout << "NEW MOVESET" << std::endl;
			}
			//exit(0);
		}

	}


}


//default, needs updating
//NOT OPTIMAL pattern
void Lieutenant::DoFormation(vec2 dir)
{
	vec2 initUnitPos = goal;
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
	sint4 marineOffset = 16;//distance between each marine in the row
	sint4 tankOffset = 25;//distance between the tanks in the row
	sint4 unitOffset = 14;//space between rows
	sint4 frontLine = 25;//distance of frontline from LT

	sint4 displace = 1;



	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		//marine.MoveTo(location, marine.GetMaxSpeed());
		//set first unit
		if(i == 0 && marine.IsAlive())
		{
			initUnitPos = vec2((ltDirection.x * frontLine) + goal.x, (ltDirection.y * frontLine) + goal.y);
			marines[i].SetGoal(Movement::Vec2D(initUnitPos.x, initUnitPos.y));
			marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));

		}
		else if(i % 2 == 1 && i > 0 && marine.IsAlive())
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2((rowDirection.x * marineOffset * displace) + initUnitPos.x, (rowDirection.y * marineOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			marines[i].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
			marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));

		}
		//set every second unit (ones on negative plane to LT) to negative
		else if(i % 2 == 0 && i > 0 && marine.IsAlive())
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2(((-rowDirection.x) * marineOffset * displace) + initUnitPos.x, ((-rowDirection.y) * marineOffset * displace) + initUnitPos.y);
			unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			unitPos.y = unitPos.y - (unitDisplace * unitOffset);
			marines[i].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
			marines[i].SetTask(mc->moveUnit(marine.GetGameObj(),  marines[i].GetGoal()));

			displace++;
		}
	}

	unitDisplace = 1;
	displace = 1;

	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		if(j == 0 && tank.IsAlive())
		{
			initUnitPos = vec2(goal.x, goal.y);
			tanks[j].SetGoal(Movement::Vec2D(initUnitPos.x, initUnitPos.y));
			tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
		}
		else if(j % 2 == 1 && j > 0 && tank.IsAlive())
		{
			//set the next marine at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2((rowDirection.x * tankOffset * displace) + initUnitPos.x, (rowDirection.y * tankOffset * displace) + initUnitPos.y);
			//unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			//unitPos.y = unitPos.y - (unitDisplace * unitOffset);

			tanks[j].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
			tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
		}
		//set every second unit (ones on negative plane to LT) to negative
		else if(j % 2 == 0 && j > 0 && tank.IsAlive())
		{
			//set the next tank at a distance based on the row direction, offset, and where the previous one is
			unitPos = vec2((-rowDirection.x * tankOffset * displace) + initUnitPos.x, (-rowDirection.y * tankOffset * displace) + initUnitPos.y);
			//unitPos.x = unitPos.x - (unitDisplace * unitOffset);
			//unitPos.y = unitPos.y - (unitDisplace * unitOffset);

			tanks[j].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
			tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
			displace++;
		}
	}

	INIT_FLAG = true;
}

//default, needs updating
void Lieutenant::MoveTo(vec2 target)
{
	location = target;
	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		marines[i].SetGoal(Movement::Vec2D(location.x, location.y));
		marines[i].SetTask(mc->moveUnit(marine.GetGameObj(),  marines[i].GetGoal()));
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);

		tanks[j].SetGoal(Movement::Vec2D(location.x, location.y));
		tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(),  tanks[j].GetGoal()));
	}
}

void Lieutenant::FireAtWill(Vector<Unit> enemies)
{
	for(size_t i(0); i<marines.size(); ++i)
	{
		Unit & marine(marines[i]);

		// Cache the unit's position and the range of its weapon
		const vec2	position(marine.GetPosition());
		const sint4 range(marine.GetWeaponRange());
		const sint4 rangeSq(range*range);

		// Choose the first enemy unit we find in range
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				marine.Attack(enemy);
				break;
			}
		}
	}
	for(size_t i(0); i<tanks.size(); ++i)
	{
		Unit & tank(tanks[i]);

		// Cache the unit's position and the range of its weapon
		const vec2	position(tank.GetPosition());
		const sint4 range(tank.GetWeaponRange());
		const sint4 rangeSq(range*range);

		// Choose the first enemy unit we find in range
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				tank.Attack(enemy);
				break;
			}
		}
	}
}

Unit Lieutenant::AquireWeakestTarget(Vector<Unit> enemies)
{
	// Cache the unit's position and the range of its weapon
	const vec2	position(GetLocation());
	const sint4 range(MAX_RANGE);
	const sint4 rangeSq(range*range);

	sint4 weakestHP = 150;	//tanks maxhealth
	Unit& target = enemies[0];

	for(size_t j(0); j<enemies.size(); ++j)
	{
		const Unit & enemy(enemies[j]);
		if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
		{
			sint4 hp = enemy.GetHitpoints();
			if (hp < weakestHP && hp > 0)
			{
				weakestHP = hp;
				target = enemy;
			}
		}
	}
	return target;
}

void Lieutenant::AttackTarget(Unit& target)
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		marine.Attack(target);
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		tank.Attack(target);
	}
}

void Lieutenant::Loop(Movement::Context& MC,Vector<Unit> enemies)
{
	mc = &MC;
	UpdateEngaged();
	if (IsEngaged())
	{
		CasualtyCheck();
	}
	FireAtWill(enemies);

	if(INIT_FLAG)
		CheckFormation();

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

