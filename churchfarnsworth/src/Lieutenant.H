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
	void RelieveUnit(sint4 type, size_t index);						//Remove a Unit from the control of the Lieutenant

	void CheckEngaged();											//Updates the engagement status of the squad
	void CheckCasualties();											//Updates the Lieutenant's squad by removing dead units
	void CheckObjective();											//checks to see if the Lieutenant has reached his goal
	void CheckFormation();											//checks to see if the units of the Lieutenant are in their correct position

	bool NeedsAid();												//Returns true if the Squad requests aid
	bool IsHealthy();												//returns if the Lieutenants health is larger than HEALTHY_VALUE;
	bool IsEngaged();												//Returns true if the Squad is in combat

	void SetAid(bool aid);											//Sets the Squad Aid status
	void SetGoal(vec2 g);											//sets goal position of the Lieutenant.
	void SetEnemies(Vector<Unit> enemies);							//set the enemy vector in the Lt
	void SetHasOrder(bool order);
	bool HasOrder();
	vec2 GetGoal();													//get the goal location of the Lieutenant.
	vec2 GetCurrentPosition();										//gets Lieutenant's position based on the average location of its units
	sint4 GetHealth();												//Returns the percentage of total Squad health
	sint4 MarineSize(){return marines.size();}						//Returns the number of Marines under the Lieutenants command
	sint4 TankSize(){return tanks.size();} 							//Returns the number of Tanks under the Lieutenants command
	vec2 GetDirection(){return ltDir;}								//Returns the facing direction of the Lieutenant's formation

	void Loop(Movement::Context&, Vector<Unit> enemies);			//The Lieutenant's game loop, all logic should be done here. Is called once every iteration of the game loop.
	void DoFormation(vec2);											//Does the Squad formation
	void MoveTo(vec2 location, vec2 facing);						//Moves the Squad to target location, doing a formation facing the direction
	Unit AquireWeakestTarget(Vector<Unit> enemies);					//Orders the Lieutenant to Find the weakest possible enemy
	void AquireTargets(Vector<Unit> enms);							//if the squad is being attacked divy up the units to attack all the enemies that are engaged
	void AttackTarget(Unit& target);								//Orders the Lieutenant's Units to Attack the given Target
	void FireAtWill(Vector<Unit> enemies);							//Orders the Lieutenant's Units to Fire at will
	void PullBackWounded();											//micro-manages units in danger of dying in the squad.
	Vector<Unit> TransferSquad();									//Transfers out the Lieutenant's squad, returning them as a vector and releaving control.
	vec2 FaceTarget(vec2 targetLocation);							//Returns the directional vector of the difference between the Lieutenant's position and it's target

private:
	Vector <Unit> marines, tanks, enemies;
	vec2 goal;
	bool hasOrder;
	vec2 ltDir;
	vec2 ltPosition;
	bool engaged;
	bool requestsAid;
	Movement::Context *mc;

};

#endif /* LIEUTENANT_H_ */
