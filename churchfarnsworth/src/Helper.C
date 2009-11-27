#include "Helper.H"
#include "Application.H"

#include "Game.H"
#include "GameObj.H"

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

Unit::Unit(Application & app, ScriptObj * object) : 
	app(app),
	unit(*object->get_GameObj()),
	weapon(unit.component("weapon")) 
{
	
}

bool Unit::IsOwned() const
{
	const Game & game(app.GetGame());
	return *unit.sod.owner == game.get_client_player();
}

bool Unit::IsEnemy() const
{
	const Game & game(app.GetGame());
	const sint4 owner(*unit.sod.owner);
	return owner != game.get_client_player() && owner != game.get_player_num();
}

bool Unit::IsCritter() const
{
	const Game & game(app.GetGame());
	return *unit.sod.owner == game.get_player_num();
}

bool Unit::IsAlive() const
{
	return unit.sod.in_game && !unit.is_dead();
}

bool Unit::IsMoving() const
{
	if(task)
	{
		Movement::Task::Status status(task->getStatus());
		return status == Movement::Task::PLANNING || status == Movement::Task::RUNNING;
	}
	else
	{
		return GetSpeed() != 0;
	}
}

bool Unit::CanMove() const
{
	return GetMaxSpeed() > 0;
}

sint4 Unit::GetMaxSpeed() const 
{ 
	return CheckPtr(unit.sod.max_speed,0); 
}

sint4 Unit::GetMaxHitpoints() const
{
	return CheckObjInt(&unit,"max_hp",0);
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
	unit.get_center(position.x,position.y);
	return position;
}

sint4 Unit::GetSpeed() const 
{ 
	return CheckPtr(unit.sod.speed,0); 
}

sint4 Unit::GetHitpoints() const
{
	return CheckObjInt(&unit,"hp",0);
}

real8 Unit::GetDistanceTo(const Unit & other) const
{
	return unit.distance_to(other.unit);
}

// Actions are specified by calling set_action on a ScriptObj
// Actions like movement are called on the GameObj itself (which is a subclass of ScriptObj)
// Actions like attacking are called on a component of the GameObj, in this case, the weapon

void Unit::Stop()
{
	// If we have a pathing task, cancel it
	if(task)
	{
		task->cancel();
		task.reset();
	}

	// Tell unit to move to current location with speed zero
	Vector<sint4> args;
	const vec2 position(GetPosition());
	args.push_back(position.x);
	args.push_back(position.y);
	args.push_back(0);
	unit.set_action("move", args);
}

void Unit::MoveTo(const vec2 & location, sint4 speed)
{
	Stop();

	Vector<sint4> args;
	args.push_back(location.x);
	args.push_back(location.y);
	args.push_back(speed);
	unit.set_action("move", args);
}

void Unit::PathTo(Movement::Goal::const_ptr goal)
{
	Stop();

	task = app.GetMoveContext().moveUnit(&unit, goal);
}

void Unit::Attack(const Unit & target)
{
	if(!weapon) return;

	Vector<sint4> args;
	args.push_back(app.GetGame().get_cplayer_info().get_id(&target.unit));
	weapon->set_action("attack", args);
}
