/*
 * Lieutenant.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

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
	GetLocation()
};

#endif /* LIEUTENANT_H_ */
