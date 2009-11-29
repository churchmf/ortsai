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
#include "General.H"

class Captain {
public:
	Captain(General& theGeneral);
	virtual ~Captain();

	void Loop();												//The Captains game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetLieutenants(Vector<Lieutenant*> theLieutenants);	//Sets the Lieutenants for the Captain to command
	vec2 GetAidRequestLocation();								//Returns the first Lieutenant's position with a request
	bool existsAidRequest();									//Checks for an aid requests
private:
	Vector<Lieutenant*> Lieutenants;							//Represents the Lieutenants under the Captains Command

	General* general;											//Represents the Captain's General
};

#endif /* CAPTAIN_H_ */
