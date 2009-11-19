/*
 * Captain.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#ifndef CAPTAIN_H_
#define CAPTAIN_H_

class Captain {
public:
	Captain();
	virtual ~Captain();

	void Loop();	//The Captains game loop, all logic should be done here. Is called once every iteration of the game loop.
};

#endif /* CAPTAIN_H_ */
