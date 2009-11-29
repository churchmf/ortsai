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

const sint4 MARINE_RANGE = 64;
const sint4 MARINE_HEALTH = 80;
const sint4 MARINE_MIN_DMG = 5;
const sint4 MARINE_MAX_DMG = 7;

const sint4 TANK_RANGE = 112;
const sint4 TANK_HEALTH = 150;
const sint4 TANK_MIN_DMG = 26;
const sint4 TANK_MAX_DMG = 34;

//maximum number of marines and tanks per lieutenant squad
const sint4 MAX_MARINES = 10;
const sint4 MAX_TANKS = 4;

const sint4 MAX_RANGE = 112;

//unit health percentage buffer for pulling back
const real8 EXPECTED_DEATH = 30;

//USED WITH SQUAD FORMATION
const sint4 UNIT_DISPLACE = 0;//variable used to displace the rows evenly
//NOTE: this variable is used do determine the distance of the row
//from the LT. so if more units get added, this can just be
//incremented for every for loop and each subsiquent row will
//be 'UNIT_OFFSET' closer to the LT for the previous row.
const sint4 MARINE_OFFSET = 16;//distance between each marine in the row
const sint4 TANK_OFFSET = 25;//distance between the tanks in the row
const sint4 UNIT_OFFSET = 10;//space between rows
const sint4 FRONT_LINE = 17;//distance of FRONT_LINE from LT

//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////

Lieutenant::Lieutenant()
{
	engaged = 0;
	requestsAid = 0;
	INIT_FLAG = false;
}

Lieutenant::~Lieutenant()
{
	delete mc;
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

Vector<Unit> Lieutenant::TransferSquad()
{
	Vector<Unit> units;
	for (size_t i(0); i < marines.size(); ++i)
		{
			Unit & marine(marines[i]);
			units.push_back(marine);
		}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		units.push_back(tank);
	}
	marines.clear();
	tanks.clear();
	return units;
}

sint4 Lieutenant::GetHealth()
{
	sint4 health = 0;
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		if (marine.IsAlive())
		{
			health += marine.GetHitpoints();
		}
	}
	for (size_t i(0); i < tanks.size(); ++i)
	{
		const Unit & tank(tanks[i]);
		if (tank.IsAlive())
		{
			health += tank.GetHitpoints();
		}
	}
	if (health == 0)
		return 0;
	else
	{
		sint4 percent = 100*(real8)((real8)health / (real8)(MAX_MARINES*80 + MAX_TANKS*150));
		return  percent;
	}
}

bool Lieutenant::IsEngaged()
{
	return engaged;
}

void Lieutenant::UpdateEngaged()
{
	engaged = false;
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		if (marine.InCombat() && marine.IsAlive())
		{
			engaged = true;
			return;
		}
	}
	for (size_t i(0); i < tanks.size(); ++i)
	{
		const Unit & tank(tanks[i]);
		if (tank.InCombat() && tank.IsAlive())
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

void Lieutenant::RelieveUnit(sint4 type, size_t index)
{
	//std::cout << "INDEX: " << index << std::endl;
	if (type == MARINE)
	{
		//std::cout << "MARINES:" << marines.size() << std::endl;
		if (index < marines.size())
		{
			marines.erase(marines.begin()+index);
		}
	}
	else if (type == TANK)
	{
		//std::cout << "TANKS: " << tanks.size() << std::endl;
		if (index < tanks.size())
		{
			tanks.erase(tanks.begin()+index);
		}
	}
}

void Lieutenant::CasualtyCheck()
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		if (!marine.IsAlive())
		{
			RelieveUnit(MARINE, i);
			break;
		}
	}
	for (size_t i(0); i < tanks.size(); ++i)
	{
		const Unit & tank(tanks[i]);
		if (!tank.IsAlive())
		{
			RelieveUnit(TANK, i);
			break;
		}
	}
}

/**
 * Micromanages each unit in the squad, checking if they are at risk of dying and moving them away from danger
 * This behavior is similar to unit dancing in standard RTS strategies, as it improves unit preservation
 * and impedes enemy focus firing.
 */
void Lieutenant::PullBackWounded()
{
	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		real8 hp = marine.GetHitpoints();
		real8 maxHp = marine.GetMaxHitpoints();
		real8 health = (hp/maxHp);
		if (health < EXPECTED_DEATH && marine.InCombat())
		{
			//move the opposite way that the damage is coming
			vec2 pullBack = vec2(-marine.DmgDirection().rX,-marine.DmgDirection().rY);
			//calculate where the unit should move and
			//create a goal. Pass it to the units movement, don't overide it's current goal as it will want to rejoin formation after microing away
			Movement::Goal::const_ptr goal = Movement::TouchPoint(Movement::Vec2D((sint4)(marine.GetPosition().x + pullBack.rX),(sint4)(marine.GetPosition().y + pullBack.rY)));
			marine.SetTask(mc->moveUnit(marine.GetGameObj(), goal));
			//std::cout << "MARINE PULLING BACK TO: "<< marine.GetPosition().x + pullBack.rX << "," << marine.GetPosition().y + pullBack.rY << std::endl;
		}
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);
		real8 hp = tank.GetHitpoints();
		real8 maxHp = tank.GetMaxHitpoints();
		real8 health = (hp/maxHp);
		if (health < EXPECTED_DEATH && tank.InCombat())
		{
			//move the opposite way that the damage is coming
			vec2 pullBack = vec2(-tank.DmgDirection().rX,-tank.DmgDirection().rY);
			//calculate where the unit should move and
			//create a goal. Pass it to the units movement, don't overide it's current goal as it will want to rejoin formation after microing away
			Movement::Goal::const_ptr goal = Movement::TouchPoint(Movement::Vec2D((sint4)(tank.GetPosition().x + pullBack.rX),(sint4)(tank.GetPosition().y + pullBack.rY)));
			tank.SetTask(mc->moveUnit(tank.GetGameObj(), goal));
		}
	}
}

vec2 Lieutenant::GetCurrentPosition()
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
	if (squadSize > 0)
	{
		vec2 position = vec2(sumLocation.x/squadSize, sumLocation.y/squadSize);
		return position;
	}
	else
		return sumLocation;
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
		//same for tank
		if(((*marines[i].GetTask()).getStatus() == Movement::Task::MOVE_FAILURE) ||
		   (((*marines[i].GetTask()).getStatus() == Movement::Task::SUCCESS) &&
		   (marines[i].IsAlive())))
		{
			if(!(((int)marines[i].GetPosition().x == (int)marines[i].GetVector().x) &&
			     ((int)marines[i].GetPosition().y == (int)marines[i].GetVector().y)))
			{
				(*marines[i].GetTask()).cancel();
				marines[i].SetTask(mc->moveUnit(marines[i].GetGameObj(), marines[i].GetGoal()));
			}
		}
	}

	for (size_t i(0); i < tanks.size(); ++i)
	{
		if(((*tanks[i].GetTask()).getStatus() == Movement::Task::MOVE_FAILURE) ||
		   (((*tanks[i].GetTask()).getStatus() == Movement::Task::SUCCESS)      &&
		   (tanks[i].IsAlive())))
		{
			if(!(((int)tanks[i].GetPosition().x == (int)tanks[i].GetVector().x) &&
				 ((int)tanks[i].GetPosition().y == (int)tanks[i].GetVector().y)))
			{
				(*tanks[i].GetTask()).cancel();
				tanks[i].SetTask(mc->moveUnit(tanks[i].GetGameObj(), tanks[i].GetGoal()));
			}
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

        ltDirection.x = ltDirection.x/sqrt(ltDirection.x*ltDirection.x + ltDirection.y*ltDirection.y);
        ltDirection.y = ltDirection.y/sqrt(ltDirection.x*ltDirection.x + ltDirection.y*ltDirection.y);

        vec2 rowDirection = vec2(-ltDirection.y, ltDirection.x);//direction of the rows

        sint4 displace = 1;

        for (size_t i(0); i < marines.size(); ++i)
        {
                Unit & marine(marines[i]);
                //marine.MoveTo(location, marine.GetMaxSpeed());
                //set first unit
                if(i == 0 && marine.IsAlive())
                {
                        initUnitPos = vec2((ltDirection.x * FRONT_LINE) + goal.x, (ltDirection.y * FRONT_LINE) + goal.y);
                        marines[i].SetGoal(Movement::Vec2D(initUnitPos.x, initUnitPos.y));
                        marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));

                }
                else if(i % 2 == 1 && i > 0 && marine.IsAlive())
                {
                        //set the next marine at a distance based on the row direction, offset, and where the previous one is
                        unitPos = vec2((rowDirection.x * MARINE_OFFSET * displace) + initUnitPos.x, (rowDirection.y * MARINE_OFFSET * displace) + initUnitPos.y);
                        unitPos.x = unitPos.x - (UNIT_DISPLACE * UNIT_OFFSET);
                        unitPos.y = unitPos.y - (UNIT_DISPLACE * UNIT_OFFSET);
                        marines[i].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
                        marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));

                }
                //set every second unit (ones on negative plane to LT) to negative
                else if(i % 2 == 0 && i > 0 && marine.IsAlive())
                {
                        //set the next marine at a distance based on the row direction, offset, and where the previous one is
                        unitPos = vec2(((-rowDirection.x) * MARINE_OFFSET * displace) + initUnitPos.x, ((-rowDirection.y) * MARINE_OFFSET * displace) + initUnitPos.y);
                        unitPos.x = unitPos.x - (UNIT_DISPLACE * UNIT_OFFSET);
                        unitPos.y = unitPos.y - (UNIT_DISPLACE * UNIT_OFFSET);
                        marines[i].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
                        marines[i].SetTask(mc->moveUnit(marine.GetGameObj(),  marines[i].GetGoal()));

                        displace++;
                }
        }

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
                        unitPos = vec2((rowDirection.x * TANK_OFFSET * displace) + initUnitPos.x, (rowDirection.y * TANK_OFFSET * displace) + initUnitPos.y);

                        tanks[j].SetGoal(Movement::Vec2D(unitPos.x, unitPos.y));
                        tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
                }
                //set every second unit (ones on negative plane to LT) to negative
                else if(j % 2 == 0 && j > 0 && tank.IsAlive())
                {
                        //set the next tank at a distance based on the row direction, offset, and where the previous one is
                        unitPos = vec2((-rowDirection.x * TANK_OFFSET * displace) + initUnitPos.x, (-rowDirection.y * TANK_OFFSET * displace) + initUnitPos.y);

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
	goal = target;
	for (size_t i(0); i < marines.size(); ++i)
	{
		Unit & marine(marines[i]);
		marines[i].SetGoal(Movement::Vec2D(goal.x, goal.y));
		marines[i].SetTask(mc->moveUnit(marine.GetGameObj(),  marines[i].GetGoal()));
	}
	for (size_t j(0); j < tanks.size(); ++j)
	{
		Unit & tank(tanks[j]);

		tanks[j].SetGoal(Movement::Vec2D(goal.x, goal.y));
		tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(),  tanks[j].GetGoal()));
	}
}

void Lieutenant::FireAtWill(Vector<Unit> enemies)
{
	for(size_t i(0); i<marines.size(); ++i)
	{
		sint4 weakestHP = 150;	//tanks maxhealth
		Unit& marine(marines[i]);
		Unit& target = enemies[0];

		// Cache the unit's position and the range of its weapon
		const vec2	position(marine.GetPosition());
		const sint4 range(marine.GetWeaponRange());
		const sint4 rangeSq(range*range);

		// Choose the weakest enemy unit we find in range
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				sint4 hp = enemy.GetHitpoints();
				if (hp <= weakestHP && hp > 0)
				{
					weakestHP = hp;
					target = enemy;
				}
			}
		}
		if(position.GetDistanceSqTo(target.GetPosition()) <= rangeSq)
		{
		marine.Attack(target);
		}
	}

	for(size_t i(0); i<tanks.size(); ++i)
	{
		sint4 weakestHP = 150;	//tanks maxhealth
		Unit& tank(tanks[i]);
		Unit& target = enemies[0];

		// Cache the unit's position and the range of its weapon
		const vec2	position(tank.GetPosition());
		const sint4 range(tank.GetWeaponRange());
		const sint4 rangeSq(range*range);

		// Choose the weakest enemy unit we find in range
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				sint4 hp = enemy.GetHitpoints();
				if (hp <= weakestHP && hp > 0)
				{
					weakestHP = hp;
					target = enemy;
				}
			}
		}
		if(position.GetDistanceSqTo(target.GetPosition()) <= rangeSq)
		{
		tank.Attack(target);
		}
	}
}

//we may not need this, it could be better just to have individual units target the weakest unit they can instead of the whole group
Unit Lieutenant::AquireWeakestTarget(Vector<Unit> enemies)
{
	// Cache the squad's position and the max range of it's weapons
	const vec2	position(GetCurrentPosition());
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
	FireAtWill(enemies);

	if(INIT_FLAG)
	{
		CheckFormation();
	}

	if (IsEngaged())
	{
		//PullBackWounded();	has issues with your CheckFormation() since I'm passing the units a movement. See PullBackWounded()

		//Unit target = AquireWeakestTarget(enemies);	see comment above function
		//AttackTarget(target);
	}
	UpdateEngaged();
	CasualtyCheck();		//I've looked over it and I have no idea why we would get a floating point exception (usually around the last unit of type in squad)

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

