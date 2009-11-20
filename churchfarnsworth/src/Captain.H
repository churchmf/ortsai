/*
 * Captain.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#ifndef CAPTAIN_H_
#define CAPTAIN_H_

#include "Helper.H"
#include "Lieutenant.H"

class Captain {
public:
	Captain();
	virtual ~Captain();

	void Loop(Vector<Lieutenant*> theLieutenants);				//The Captains game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetLieutenants(Vector<Lieutenant*> theLieutenants);	//Sets the Lieutenants for the Captain to command
private:
	Vector<Lieutenant*> Lieutenants;	//Represents the Lieutenants under the Captains Command
};

#endif /* CAPTAIN_H_ */
