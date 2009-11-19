/*
 * Lieutenant.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */
#ifndef LIEUTENANT_H_
#define LIEUTENANT_H_

#include "Helper.H"


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
	
private:
	Vector <Unit> marines, tanks;
	sint4 health;
	vec2 location;
	bool engaged;
};

#endif /* LIEUTENANT_H_ */
