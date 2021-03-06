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

//marine values, should match actual values
const sint4 MARINE_RANGE = 64;
const sint4 MARINE_HEALTH = 80;
const sint4 MARINE_MIN_DMG = 5;
const sint4 MARINE_MAX_DMG = 7;

//tanks values, should match actual values
const sint4 TANK_RANGE = 112;
const sint4 TANK_HEALTH = 150;
const sint4 TANK_MIN_DMG = 26;
const sint4 TANK_MAX_DMG = 34;

//should be longest unit range in squad, in this case tank range
const sint4 MAX_RANGE = 112;
//
const sint4 MAX_ENEMY_DIST = 180;

//maximum number of marines and tanks per lieutenant squad
//should match same-named constants in Captain.C
const sint4 MAX_MARINES = 10;
const sint4 MAX_TANKS = 4;


//UNIT MICROMANAGEMENT
//unit health percentage buffer for pulling back
const real8 EXPECTED_DEATH = 50;
//distance to pull back the unit
const real8 PULL_BACK_BUFFER = 5;

//SQUAD FORMATION
const sint4 UNIT_DISPLACE = 0;//variable used to displace the rows evenly
//NOTE: this variable is used do determine the distance of the row
//from the LT. so if more units get added, this can just be
//incremented for every for loop and each subsiquent row will
//be 'UNIT_OFFSET' closer to the LT for the previous row.
const sint4 MARINE_OFFSET = 16;//distance between each marine in the row
const sint4 TANK_OFFSET = 20;//distance between the tanks in the row
const sint4 UNIT_OFFSET = 10;//space between rows
const sint4 FRONT_LINE = 17;//distance of FRONT_LINE from LT

//Value to determine if the Lieutenant is Healthy (percentage)
const sint4 HEALTHY_VALUE = 30;

//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////


Lieutenant::Lieutenant()
{
	engaged = 0;
	requestsAid = 0;
}

Lieutenant::~Lieutenant()
{
	delete mc;
}

bool Lieutenant::IsHealthy()
{
	if (GetHealth() >= HEALTHY_VALUE)
	{
		return true;
	}
	return false;
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
	//zero unit check
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

void Lieutenant::CheckEngaged()
{
	engaged = false;
	for (size_t i(0); i < marines.size(); ++i)
	{
		const Unit & marine(marines[i]);
		// Cache the unit's position and the range of its weapon
		const vec2	position(marine.GetPosition());
		const sint4 range(marine.GetWeaponRange());
		const sint4 rangeSq(range*range);
		//Quick Check
		if (marine.InCombat() && marine.IsAlive())
		{
			engaged = true;
			return;
		}
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				engaged = true;
				return;
			}
		}
	}

	for (size_t i(0); i < tanks.size(); ++i)
	{
		const Unit & tank(tanks[i]);
		// Cache the unit's position and the range of its weapon
		const vec2	position(tank.GetPosition());
		const sint4 range(tank.GetWeaponRange());
		const sint4 rangeSq(range*range);
		//Quick Check
		if (tank.InCombat() && tank.IsAlive())
		{
			engaged = true;
			return;
		}
		for(size_t j(0); j<enemies.size(); ++j)
		{
			const Unit & enemy(enemies[j]);
			if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
			{
				engaged = true;
				return;
			}
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

void Lieutenant::CheckCasualties()
{
	if(marines.empty() && tanks.empty())
	{
		return;
	}
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
			vec2 pullBack = vec2(-PULL_BACK_BUFFER*marine.DmgDirection().rX,-PULL_BACK_BUFFER*marine.DmgDirection().rY);
			//calculate where the unit should move and
			//create a goal. Pass it to the units movement.
			(*(marine.GetTask())).cancel();
			marine.SetGoal(Movement::Vec2D((marine.GetPosition().x + pullBack.rX),(marine.GetPosition().y + pullBack.rY)));
			marine.SetTask(mc->moveUnit(marine.GetGameObj(), marine.GetGoal()));
			//std::cout << "MARINE PULLING BACK: "<< marine.GetVector().x - marine.GetPosition().x << "," << marine.GetVector().y - marine.GetPosition().y << std::endl;
		}
	}
	for (size_t i(0); i < tanks.size(); ++i)
	{
		Unit & tank(tanks[i]);
		real8 hp = tank.GetHitpoints();
		real8 maxHp = tank.GetMaxHitpoints();
		real8 health = (hp/maxHp);
		if (health < EXPECTED_DEATH && tank.InCombat())
		{
			//move the opposite way that the damage is coming
			vec2 pullBack = vec2(-PULL_BACK_BUFFER*tank.DmgDirection().rX,-PULL_BACK_BUFFER*tank.DmgDirection().rY);
			//calculate where the unit should move and
			//create a goal. Pass it to the units movement
			(*(tank.GetTask())).cancel();
			tank.SetGoal(Movement::Vec2D((tank.GetPosition().x + pullBack.rX),(tank.GetPosition().y + pullBack.rY)));
			tank.SetTask(mc->moveUnit(tank.GetGameObj(), tank.GetGoal()));
			//std::cout << "TANK PULLING BACK : "<< tank.GetVector().x - tank.GetPosition().x << "," << tank.GetVector().y - tank.GetPosition().y << std::endl;
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
		ltPosition = position;
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

bool Lieutenant::HasOrder()
{
	return hasOrder;
}

void Lieutenant::SetHasOrder(bool order)
{
	hasOrder = order;
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

//NOT OPTIMAL pattern
void Lieutenant::DoFormation(vec2 dir)
{
        vec2 initUnitPos = goal;
        ltDir = dir;
        vec2 unitPos = initUnitPos;
        vec2 ltDirection = dir;

        ltDirection.rX = dir.rX/sqrt(dir.rX*dir.rX + dir.rY*dir.rY);
        ltDirection.rY = dir.rY/sqrt(dir.rX*dir.rX + dir.rY*dir.rY);

		vec2 rowDirection = vec2(-ltDirection.rY, ltDirection.rX);//direction of the rows

        sint4 displace = 1;

        for (size_t i(0); i < marines.size(); ++i)
        {
                Unit & marine(marines[i]);
                //set first unit
                if(i == 0 && marine.IsAlive())
                {
					initUnitPos = vec2((ltDirection.rX * (real8)FRONT_LINE) + (real8)goal.x, (ltDirection.rY * (real8)FRONT_LINE) + (real8)goal.y);

					marines[i].SetGoal(Movement::Vec2D((sint4)initUnitPos.rX, (sint4)initUnitPos.rY));
					marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));
                }
                else if(i % 2 == 1 && i > 0 && marine.IsAlive())
                {
					//set the next marine at a distance based on the row direction, offset, and where the previous one is
					unitPos = vec2((rowDirection.rX * (real8)MARINE_OFFSET * (real8)displace) + initUnitPos.rX,
								   (rowDirection.rY * (real8)MARINE_OFFSET * (real8)displace) + initUnitPos.rY);
					unitPos.rX = unitPos.rX - ((real8)UNIT_DISPLACE * (real8)UNIT_OFFSET);
					unitPos.rY = unitPos.rY - ((real8)UNIT_DISPLACE * (real8)UNIT_OFFSET);
					marines[i].SetGoal(Movement::Vec2D((sint4)unitPos.rX, (sint4)unitPos.rY));
					marines[i].SetTask(mc->moveUnit(marine.GetGameObj(), marines[i].GetGoal()));

                }
                //set every second unit (ones on negative plane to LT) to negative
                else if(i % 2 == 0 && i > 0 && marine.IsAlive())
                {
					//set the next marine at a distance based on the row direction, offset, and where the previous one is
					unitPos = vec2(((-rowDirection.rX) * (real8)MARINE_OFFSET * (real8)displace) + initUnitPos.rX,
								   ((-rowDirection.rY) * (real8)MARINE_OFFSET * (real8)displace) + initUnitPos.rY);
					unitPos.x = unitPos.rX - ((real8)UNIT_DISPLACE * (real8)UNIT_OFFSET);
					unitPos.y = unitPos.rY - ((real8)UNIT_DISPLACE * (real8)UNIT_OFFSET);
					marines[i].SetGoal(Movement::Vec2D((sint4)unitPos.rX, (sint4)unitPos.rY));
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
					initUnitPos = vec2((real8)goal.x, (real8)goal.y);
					unitPos = vec2((-rowDirection.rX * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rX,
							       (-rowDirection.rY * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rY);

					tanks[j].SetGoal(Movement::Vec2D((sint4)unitPos.rX, (sint4)unitPos.rY));
					tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
                }
                else if(j % 2 == 1 && j > 0 && tank.IsAlive())
                {
					//set the next tank at a distance based on the row direction, offset, and where the previous one is
					unitPos = vec2((rowDirection.rX * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rX,
								   (rowDirection.rY * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rY);

					tanks[j].SetGoal(Movement::Vec2D((sint4)unitPos.rX, (sint4)unitPos.rY));
					tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
                }
                //set every second unit (ones on negative plane to LT) to negative
                else if(j % 2 == 0 && j > 0 && tank.IsAlive())
                {
                	displace++;
					//set the next tank at a distance based on the row direction, offset, and where the previous one is
					unitPos = vec2((-rowDirection.rX * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rX,
							       (-rowDirection.rY * (real8)TANK_OFFSET * (real8)displace) + initUnitPos.rY);

					tanks[j].SetGoal(Movement::Vec2D((sint4)unitPos.rX, (sint4)unitPos.rY));
					tanks[j].SetTask(mc->moveUnit(tank.GetGameObj(), tanks[j].GetGoal()));
                }
        }
}

//default, needs updating
void Lieutenant::MoveTo(vec2 target, vec2 direction)
{
	goal = target;
	//ClearSquadOrders(); doesn't fix the tank not retreating problem
	DoFormation(direction);
	hasOrder = true;
}

void Lieutenant::ClearSquadOrders()
{
	for(size_t i(0); i<marines.size(); ++i)
	{
		(*marines[i].GetTask()).cancel();
	}
	for(size_t i(0); i<tanks.size(); ++i)
	{
		(*tanks[i].GetTask()).cancel();
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
		engaged = true;
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
		engaged = true;
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


void Lieutenant::SetEnemies(Vector<Unit> enms)
{
	if((marines.empty() && tanks.empty()) ||enms.empty())
	{
		return;
	}
	sint4 iDist;

	enemies.clear();

	for(size_t i(0); i< enms.size(); ++i)
	{
		iDist = (ltPosition.x - enms[i].GetPosition().x) * (ltPosition.x - enms[i].GetPosition().x) +
				(ltPosition.y - enms[i].GetPosition().y) * (ltPosition.y - enms[i].GetPosition().y);

		iDist = sqrt(iDist);

		//if the enemy is close, and it is firing at someone, add it to the target enemies vector
		if(iDist < MAX_ENEMY_DIST && enms[i].IsAlive() && enms[i].InCombat())
			enemies.push_back(enms[i]);
	}
}


void Lieutenant::AquireTargets(Vector<Unit> enms)
{
	//sort all the enemies by there distance from the lt.
	SetEnemies(enms);
	if(enemies.size() <= 0)
	{
		return;
	}

	if(enemies.size() >= marines.size() + tanks.size())
	{
		for(size_t i(0); i< enms.size(); ++i)
		{
			if(i < marines.size())
			{
				marines[i].SetGoal(Movement::Vec2D(enemies[i].GetPosition().x, enemies[i].GetPosition().y));
				marines[i].SetTask(mc->moveUnit(marines[i].GetGameObj(), marines[i].GetAttackGoal()));
			}
			if(i < tanks.size())
			{
				tanks[i].SetGoal(Movement::Vec2D(enemies[i].GetPosition().x, enemies[i].GetPosition().y));
				tanks[i].SetTask(mc->moveUnit(tanks[i].GetGameObj(), tanks[i].GetAttackGoal()));
			}
		}
	}
	else
	{
		for (size_t i(0); i < marines.size(); ++i)
		{
			marines[i].SetGoal(Movement::Vec2D(enemies[i % enemies.size()].GetPosition().x, enemies[i % enemies.size()].GetPosition().y));
			marines[i].SetTask(mc->moveUnit(marines[i].GetGameObj(), marines[i].GetAttackGoal()));
		}
		for (size_t j(0); j < tanks.size(); ++j)
		{
			tanks[j].SetGoal(Movement::Vec2D(enemies[j % enemies.size()].GetPosition().x, enemies[j % enemies.size()].GetPosition().y));
			tanks[j].SetTask(mc->moveUnit(tanks[j].GetGameObj(), tanks[j].GetAttackGoal()));
		}
	}
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

vec2 Lieutenant::FaceTarget(vec2 targetLocation)
{
	const real8 PI = 3.14159265;
	//std::cout << "target: " << targetLocation.x << "," << targetLocation.y << std::endl;
	//std::cout << "current: " << GetCurrentPosition().x << "," << GetCurrentPosition().y << std::endl;
	vec2 directionalVector = vec2(0,0);
	vec2 facing = targetLocation-GetCurrentPosition();
	real8 angle = atan2(facing.y,facing.x) * 180.0 / PI;

	real8 rX = cos(angle*(PI/180));
	real8 rY = sin(angle*(PI/180));
	directionalVector = vec2(rX,rY);

	//std::cout << "facing: " << facing.x << "," << facing.y << std::endl;
	//std::cout << "angle: " << angle << std::endl;
	//std::cout << "directionalVector: " << directionalVector.x << "," << directionalVector.y << std::endl;
	return directionalVector;
}

void Lieutenant::CheckObjective()
{
	if (hasOrder)
	{
		if  (GetCurrentPosition().IsNear(goal) )
		{
			hasOrder = false;
		}
	}
	//std::cout << hasOrder << ":" << GetCurrentPosition().x << "," << GetCurrentPosition().y <<   " vs " << goal.x  << "," << goal.y << std::endl;
}

void Lieutenant::Wait()
{
	hasOrder = true;
}

void Lieutenant::Resume()
{
	hasOrder = false;
}

///////////////////////////////////////////////////////////////
///////////////////   LIEUTENANT LOOP     ////////////////////
/////////////////////////////////////////////////////////////
////////////////////   LOW LEVEL STRATEGY  /////////////////
void Lieutenant::Loop(Movement::Context& MC,Vector<Unit> enemies)
{
	mc = &MC;
	CheckCasualties();
	//Dead lieutenant check
	if (tanks.empty() && marines.empty())
	{
		hasOrder = false;
		goal = vec2(0,0);
		engaged = false;
		requestsAid = false;
		return;
	}
	CheckEngaged();
	CheckObjective();
	CheckFormation();

	//If the lieutenant doesn't have an order
	if (!hasOrder)
	{
		//PullBackWounded(); //has mixed results, recommended testing before implementation
		FireAtWill(enemies);
		//If the lieutenant is in combat
		if (engaged)
		{
				AquireTargets(enemies);
		}
		else
		{
			//If the lieutenant is safe and healthy (maybe after reforming?)
			if (GetHealth() >= HEALTHY_VALUE)
			{
				//Sets the lieutenants aidRequest to false
				SetAid(false);
			}
		}
	}
}
///////////////////////////////////////////////////////////////
/////////////////   END LIEUTENANT LOOP   ////////////////////
/////////////////////////////////////////////////////////////

