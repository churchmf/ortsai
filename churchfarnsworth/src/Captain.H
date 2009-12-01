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
	struct LtCompare
	{
	  bool operator() (Lieutenant* i,Lieutenant* j) { return (i->MarineSize()+i->TankSize() < j->MarineSize()+j->TankSize());}
	} compLieuts;
	Captain(General& theGeneral);
	virtual ~Captain();

	void Loop(const sint4 frame);								//The Captains game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetLieutenants(Vector<Lieutenant*> theLieutenants);	//Sets the Lieutenants for the Captain to command
	vec2 GetClosestAidRequestLocation(vec2 lieutPos);			//Returns the closest Lieutenant's position with an aid request
	bool existsAidRequest();									//Checks for an aid requests in each Lieutenant
	void Deploy();												//Deploys the Lieutenants into starting formation
	void RemoveLieutenant(size_t index);							//Removes the Lieutenant at index i from the Captain's command
	void DistributeUnits(Vector<Unit> units);					//Distributes the units across the Lieutenants
	Vector<Lieutenant*> Lieutenants;							//Represents the Lieutenants under the Captains Command
	General* general;											//Represents the Captain's General
};
#endif /* CAPTAIN_H_ */
