/*
 * Lieutenant.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */
#ifndef LIEUTENANT_H_
#define LIEUTENANT_H_

#include "Helper.H"
#include "Movement.H"



class Lieutenant {
public:
	Lieutenant(GameStateModule&);
	virtual ~Lieutenant();
	void AssignUnit(Unit unit);		//Assign a Unit to the control of the Lieutenant
	bool RelieveUnit(Unit unit);	//Remove a Unit from the control of the Lieutenant, returns true if removed
	sint4 GetHealth();				//Returns the percentage of total Squad health
	bool IsEngaged();				//Returns true if the Squad is in combat
	void UpdateEngaged();			//Updates the engagement Status of the squad
	bool NeedsAid();				//Returns true if the Squad requests aid
	void SetAid(bool aid);			//Sets the Squad Aid status
	vec2 GetLocation();				//Returns a the centralized location of the squad
	void DoFormation(vec2);			//Does the Squad formation
	void MoveTo(vec2);				//Moves the Squad to target location
	void Loop(Movement::Context&);					//The Lieutenant's game loop, all logic should be done here. Is called once every iteration of the game loop.

private:
	Vector <Unit> marines, tanks;
	sint4 health;
	vec2 location;
	bool engaged;
	bool requestsAid;
	Movement::Context *mc;
};

#endif /* LIEUTENANT_H_ */
