/*
 * Lieutenant.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */
#include "Application.H"
#include "Helper.H"

#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"

#ifndef LIEUTENANT_H_
#define LIEUTENANT_H_

class Lieutenant {
public:
	Lieutenant();
	virtual ~Lieutenant();
	void AssignUnit(Unit unit);		//Assign a Unit to the control of the Lieutenant
	bool RelieveUnit(Unit unit);	//Remove a Unit from the control of the Lieutenant, returns true if removed
	sint4 GetHealth();				//Returns the total health of the Squad
	bool IsEngaged();				//Returns true if the Squad is in combat
	vec2 GetLocation();				//Returns a the centeralized location of the squad
	void DoFormation();				//Does the Squad formation
};

#endif /* LIEUTENANT_H_ */
