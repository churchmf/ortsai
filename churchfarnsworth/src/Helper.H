#ifndef ORTS_SAMPLE_HELPER_H
#define ORTS_SAMPLE_HELPER_H

#include "Global.H"	// For types
#include "Movement.H"

//////////////////////////////////////////////////////////
// Helper struct to easily manipulate positions/offsets //
//////////////////////////////////////////////////////////
struct vec2
{
	sint4 x,y;
	real8 rX, rY;

	vec2() {}
	vec2(sint4 x, sint4 y) : x(x), y(y) {}
	vec2(real8 x, real8 y) : rX(x), rY(y){}

	vec2 operator + (const vec2 & v) const { return vec2(x+v.x,y+v.y); }
	vec2 operator - (const vec2 & v) const { return vec2(x-v.x,y-v.y); }


	float GetLength() const { return sqrtf(static_cast<float>(x*x+y*y)); }
	sint4 GetLengthSq() const { return x*x+y*y; }

	float GetDistanceTo(const vec2 & v) const { return (v-*this).GetLength(); }
	sint4 GetDistanceSqTo(const vec2 & v) const { return (v-*this).GetLengthSq(); }
};


struct Color
{
	real4 r,g,b;

	Color() {}
	Color(real4 r, real4 g, real4 b) : r(r), g(g), b(b) {}
};

/////////////////////////////////////////
// Helper class to easily access units //
/////////////////////////////////////////

class Game;
class GameObj;
class ScriptObj;

class Unit
{
	const Game *	game;
	GameObj *		unit;
	ScriptObj *		weapon;
	sint4			mode;
	Movement::Task::ptr task;
	Movement::Vec2D goal;

public:
					Unit(const Game & game, ScriptObj * object);

	////////////////
	// Unit State //
	////////////////

	bool			IsAlive() const;			// Is unit alive?
	bool			IsMoving() const;			// Is unit moving?

	sint4			GetMaxSpeed() const;		// Maximum speed unit can move at
	sint4			GetMaxHitpoints() const;	// Maximum hitpoints of unit

	bool			HasWeapon() const;			// Does unit have a weapon?
	sint4			GetWeaponMinDamage() const; // Get the minimum damage for the weapon
	sint4			GetWeaponMaxDamage() const;	// Get the maximum damage for the weapon
	sint4			GetWeaponCooldown() const;	// Get the wait time after weapon is fired
	sint4			GetWeaponRange() const;		// Get the range of the weapon

	vec2			GetPosition() const;		// Get the current position of the unit
	sint4			GetSpeed() const;			// Get the current speed of the unit
	sint4			GetHitpoints() const;		// Get the current hitpoints of the unit

	bool			InCombat() const;			// Is the unit in combat?
	vec2 			DmgDirection() const;	// returns a direction vector of where the unit is being attacked
	sint4			GetType() const;			// Get the type of the unit (1=marine,2=tank)
	sint4			GetMode() const;			// Get the mode of the unit (0=weapon, 2=siege)

	void			SetTask(Movement::Task::ptr);
	Movement::Task::ptr GetTask();

	void 			SetGoal(Movement::Vec2D g);
	Movement::Goal::const_ptr GetGoal();
	Movement::Goal::const_ptr GetAttackGoal();
	Movement::Vec2D GetVector();

	//////////////
	// Commands //
	//////////////

	void MoveTo(const vec2 & location,	// Location to move to
				sint4 speed);			// Speed at which to move

	void Attack(const Unit & target);	// Unit to attack
	GameObj* GetGameObj() {return unit;}
};

#endif
