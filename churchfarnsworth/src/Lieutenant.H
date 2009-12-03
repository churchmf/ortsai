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
	void RelieveUnit(sint4 type, size_t index);						//Remove a Unit of the given type and index from the control of the Lieutenant

	void CheckEngaged();											//Checks to see if the squad is Engaged
	void CheckCasualties();											//Updates the Lieutenant's squad by removing dead units
	void CheckObjective();											//Checks to see if the Lieutenant has reached his goal
	void CheckFormation();											//Checks to see if the units of the Lieutenant are in their correct position

	bool NeedsAid();												//Returns true if the Squad requests aid
	bool IsHealthy();												//Returns if the Lieutenants health is larger than HEALTHY_VALUE;
	bool IsEngaged();												//Returns true if the Squad is in combat
	bool HasOrder();												//Returns true if the Squad is executing a given order

	void SetAid(bool aid);											//Sets the Squad Aid status
	void SetGoal(vec2 g);											//Sets goal position of the Lieutenant.
	void SetEnemies(Vector<Unit> enemies);							//Set the enemy vector in the Lt
	void SetHasOrder(bool order);									//Sets if the Lieutenant is executing an order

	vec2 GetGoal();													//Get the goal location of the Lieutenant.
	vec2 GetCurrentPosition();										//Gets Lieutenant's position based on the average location of its units
	sint4 GetHealth();												//Returns the percentage of total Squad health
	sint4 MarineSize(){return marines.size();}						//Returns the number of Marines under the Lieutenants command
	sint4 TankSize(){return tanks.size();} 							//Returns the number of Tanks under the Lieutenants command
	vec2 GetDirection(){return ltDir;}								//Returns the facing direction of the Lieutenant's formation

	void Loop(Movement::Context&, Vector<Unit> enemies);			//The Lieutenant's game loop, all logic should be done here. Is called once every iteration of the game loop.
	void DoFormation(vec2);											//Does the Squad formation
	void MoveTo(vec2 location, vec2 facing);						//Moves the Squad to target location, doing a formation facing the direction
	Unit AquireWeakestTarget(Vector<Unit> enemies);					//Orders the Lieutenant to Find the weakest possible enemy
	void AquireTargets(Vector<Unit> enms);							//If the squad is being attacked divy up the units to attack all the enemies that are engaged
	void AttackTarget(Unit& target);								//Orders the Lieutenant's Units to Attack the given Target
	void FireAtWill(Vector<Unit> enemies);							//Orders the Lieutenant's Units to Fire at will
	void PullBackWounded();											//micro-manages units in danger of dying in the squad.
	Vector<Unit> TransferSquad();									//Transfers out the Lieutenant's squad, returning them as a vector and releaving control.
	vec2 FaceTarget(vec2 targetLocation);							//Returns the directional vector of the difference between the Lieutenant's position and it's target
	void ClearSquadOrders();										//Clears the squad of all orders, should only be called if the squad already has orders
	void Resume();													//The Lieutenant returns to normal execution
	void Wait();													//The Lieutenant will wait, until told to resume

private:
	Vector <Unit> marines, tanks, enemies;							//Represents the units known to the Lieutenant including his squad and enemies
	vec2 goal;														//The goal location of a Lieutenant's orders
	bool hasOrder;													//If the Lieutenant is executing an order
	vec2 ltDir;														//The direction the Lieutenant is facing
	vec2 ltPosition;												//The position of the Lieutenant
	bool engaged;													//If the Lieutenant is in combat
	bool requestsAid;												//If the Lieutenant requests aid
	Movement::Context *mc;											//Lieutenant's Movement Context, used for pathfinding

};

#endif /* LIEUTENANT_H_ */
