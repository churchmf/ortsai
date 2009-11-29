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
	Lieutenant();
	virtual ~Lieutenant();
	void AssignUnit(Unit unit);										//Assign a Unit to the control of the Lieutenant
	void RelieveUnit(sint4 type, sint4 index);						//Remove a Unit from the control of the Lieutenant
	sint4 GetHealth();												//Returns the percentage of total Squad health
	bool IsEngaged();												//Returns true if the Squad is in combat
	void UpdateEngaged();											//Updates the engagement status of the squad
	void CasualtyCheck();											//Updates the Lieutenant's squad by removing dead units
	bool NeedsAid();												//Returns true if the Squad requests aid
	void SetAid(bool aid);											//Sets the Squad Aid status
	vec2 GetCurrentPosition();										//gets Lieutenant's position
	void DoFormation(vec2);											//Does the Squad formation
	void MoveTo(vec2);												//Moves the Squad to target location
	void Loop(Movement::Context&, Vector<Unit> enemies);			//The Lieutenant's game loop, all logic should be done here. Is called once every iteration of the game loop.
	Unit AquireWeakestTarget(Vector<Unit> enemies);					//Orders the Lieutenant to Find the weakest possible enemy
	void AttackTarget(Unit& target);								//Orders the Lieutenant's Units to Attack the given Target
	void FireAtWill(Vector<Unit> enemies);							//Orders the Lieutenant's Units to Fire at will
	void SetGoal(vec2 g);											//sets goal position of the Lieutenant.
	vec2 GetGoal();													//get the goal location of the Lieutenant.
	void CheckFormation();											//checks to see if the units of the Lieutenant are in their correct position
	void PullBackWounded();											//micro-manages units in danger of dying in the squad.
	Vector<Unit> TransferSquad();									//Transfers out the Lieutenant's squad, returning them as a vector and releaving control.

	sint4 MarineSize(){return marines.size();}	//Returns the number of Marines under the Lieutenants command
	sint4 TankSize(){return tanks.size();} 		//Returns the number of Tanks under the Lieutenants command

private:
	Vector <Unit> marines, tanks;
	vec2 goal;
	bool engaged;
	bool requestsAid;
	bool INIT_FLAG;		//flag used to test if the Lieutenant's variables have been initialized
	Movement::Context *mc;
};

#endif /* LIEUTENANT_H_ */
