#include "Helper.H"

#include "Game.H"
#include "GameObj.H"
#include "Movement.H"

// GameObj::sod contains pointers to commonly accessed data fields
// GameObj is a subclass of ScriptObj
// All ScriptObjs have data fields with can be accessed with methods like
// ScriptObj::has_attr() and ScriptObj::get_int()

template<typename T>
const T & CheckPtr(const T * pointer, const T & defValue)
{
	return pointer ? *pointer : defValue;
}

int CheckObjInt(const ScriptObj * object, const char * name, int defValue)
{
	if(!object) return defValue;
	const stype * ptr(object->get_val_ptr(name));
	return ptr ? ptr->to_int() : defValue;
}

Unit::Unit(const Game & game, ScriptObj * object) :
	game(&game),
	unit(object->get_GameObj()),
	weapon(unit->component("weapon"))
{

}

bool Unit::IsAlive() const
{
	return unit->sod.in_game && !unit->is_dead();
}

bool Unit::IsMoving() const
{
	return GetSpeed() != 0;
}

sint4 Unit::GetMaxSpeed() const
{
	return CheckPtr(unit->sod.max_speed,0);
}

sint4 Unit::GetMaxHitpoints() const
{
	return CheckObjInt(unit,"max_hp",0);
}

bool Unit::HasWeapon() const
{
	return weapon != 0;
}

sint4 Unit::GetWeaponMinDamage() const
{
	return CheckObjInt(weapon,"min_damage",0);
}

sint4 Unit::GetWeaponMaxDamage() const
{
	return CheckObjInt(weapon,"max_damage",0);
}

sint4 Unit::GetWeaponCooldown() const
{
	return CheckObjInt(weapon,"cooldown",0);
}

sint4 Unit::GetWeaponRange() const
{
	return CheckObjInt(weapon,"max_ground_range",0);
}

vec2 Unit::GetPosition() const
{
	vec2 position;
	unit->get_center(position.x,position.y);
	return position;
}

sint4 Unit::GetMode() const
{
	return *unit->get_int_ptr("mode");
}

sint4 Unit::GetSpeed() const
{
	return CheckPtr(unit->sod.speed,0);
}

sint4 Unit::GetHitpoints() const
{
	return CheckObjInt(unit,"hp",0);
}

sint4 Unit::GetType() const
{
	const sint4 unittypediv = 3;
	//returns 1 for marine and 2 for tank
	return *unit->sod.radius / unittypediv;
}

bool Unit::InCombat() const
{
	uint4 dmg = unit->dir_dmg;
	return dmg!=0;
}

/**
 * Returns the direction of an enemy attacker into a vec2(x,y) where sqrt(x*x+y*y) is a direct line to the attacker.
 */
vec2 Unit::DmgDirection() const
{
	const real8 PI = 3.14159265;
	//converts the dmg bit vector into a directional vector
	uint4 dmg = unit->dir_dmg;
	int digit = 0;
	while (dmg > 32)
	{
		dmg = dmg/2;
		digit++;
	}
	//(-360/32) = -11, thus every digit is an extra -11 degrees
	sint4 angle = 90 + (-11*digit);
	//bound angle to fit unit circle
	if (angle < 0)
	{
		angle += 360;
	}

	//default directionVector (no direction)
	vec2 directionVector = vec2(0,0);

	//if there is infact dmg then calculate the directionVector
	if (dmg != 0)
	{
		real8 x = cos(angle*(PI/180));
		real8 y = sin(angle*(PI/180));
		//std::cout << angle << ": " << x << "," << y << std::endl;
		directionVector = vec2(x,y);
	}
	return directionVector;
}

// Actions are specified by calling set_action on a ScriptObj
// Actions like movement are called on the GameObj itself (which is a subclass of ScriptObj)
// Actions like attacking are called on a component of the GameObj, in this case, the weapon

void Unit::MoveTo(const vec2 & location, sint4 speed)
{
	Vector<sint4> args;
	args.push_back(location.x);
	args.push_back(location.y);
	args.push_back(speed);
	unit->set_action("move", args);
}

void Unit::Attack(const Unit & target)
{
	if(!weapon) return;

	Vector<sint4> args;
	args.push_back(game->get_cplayer_info().get_id(target.unit));
	weapon->set_action("attack", args);
}

void Unit::SetTask(Movement::Task::ptr p)
{
	task = p;
}
Movement::Task::ptr Unit::GetTask()
{
	return task;
}

void Unit::SetGoal(Movement::Vec2D g)
{
	goal = g;
}
Movement::Goal::const_ptr Unit::GetGoal()
{
	return Movement::TouchPoint(goal);
}

Movement::Goal::const_ptr Unit::GetAttackGoal()
{
	return Movement::AttackPoint(goal);
}

Movement::Vec2D Unit::GetVector()
{
	return goal;
}


