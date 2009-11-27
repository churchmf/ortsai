#ifndef ORTS_SAMPLE_HELPER_H
#define ORTS_SAMPLE_HELPER_H

#include "Global.H"		// For Types
#include "Movement.H"	// For Movement:: { Context, Task, Goal, Vec2D }

typedef Movement::Vec2D vec2;

struct Color
{
	real4 r,g,b;

	Color() {}
	Color(real4 r, real4 g, real4 b) : r(r), g(g), b(b) {}
};

/////////////////////////////////////////
// Helper class to easily access units //
/////////////////////////////////////////

class Application;
class GameObj;
class ScriptObj;

class Unit
{
public:
	typedef boost::shared_ptr<Unit>			ptr;
	typedef boost::shared_ptr<Unit const>	const_ptr;
private:
	Application &		app;
	GameObj &			unit;
	ScriptObj *			weapon;
	Movement::Task::ptr	task;
public:
			Unit(Application & app, ScriptObj * object);

	/////////////////////
	// Unit properties //
	/////////////////////

	bool	IsOwned() const;			// Does unit belong to this client?
	bool	IsEnemy() const;			// Is unit an enemy of this client?
	bool	IsCritter() const;			// Is unit a neutral annoyance?

	bool	CanMove() const;			// Can object move?
	sint4	GetMaxSpeed() const;		// Maximum speed unit can move at

	sint4	GetMaxHitpoints() const;	// Maximum hitpoints of unit

	bool	HasWeapon() const;			// Does unit have a weapon?
	sint4	GetWeaponMinDamage() const; // Get the minimum damage for the weapon
	sint4	GetWeaponMaxDamage() const;	// Get the maximum damage for the weapon
	sint4	GetWeaponCooldown() const;	// Get the wait time after weapon is fired
	sint4	GetWeaponRange() const;		// Get the range of the weapon

	////////////////
	// Unit state //
	////////////////

	bool	IsAlive() const;			// Is unit alive?
	bool	IsMoving() const;			// Is unit moving?

	vec2	GetPosition() const;		// Get the current position of the unit
	sint4	GetSpeed() const;			// Get the current speed of the unit
	sint4	GetHitpoints() const;		// Get the current hitpoints of the unit

	////////////////////////
	// Helper information //
	////////////////////////

	real8	GetDistanceTo(const Unit & other) const;

	//////////////
	// Commands //
	//////////////

	void Stop();

	void MoveTo(const vec2 & location,				// Location to move to
				sint4 speed);						// Speed at which to move

	void PathTo(Movement::Goal::const_ptr goal);	// Goal to path to

	void Attack(const Unit & target);				// Unit to attack

	///////////////////////////
	// Low-level ORTS access //
	///////////////////////////

	GameObj &			GetGameObj()			{ return unit; }
	const GameObj &		GetGameObj() const		{ return unit; }
	GameObj *			GetGameObjPtr()			{ return &unit; }
	const GameObj *		GetGameObjPtr() const	{ return &unit; }
};

// Template function for quickly removing an element from a vector
template<typename T>
void quick_erase(std::vector<boost::shared_ptr<T> > & vector, 
                 const boost::shared_ptr<T> & element)
{
    for(size_t i(0); i<vector.size(); ++i)
    {
        if(vector[i] == element)
        {
            std::swap(vector[i],vector.back());
            vector.pop_back();
            return;
        }
    }
}

// Template function for quickly removing an element from a vector
template<typename T>
void quick_erase(std::vector<boost::shared_ptr<T const> > & vector, 
                 const boost::shared_ptr<T> & element)
{
    for(size_t i(0); i<vector.size(); ++i)
    {
        if(vector[i] == element)
        {
            std::swap(vector[i],vector.back());
            vector.pop_back();
            return;
        }
    }
}

#endif
