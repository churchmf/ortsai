/*
 * General.H
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include "Helper.H"

class General {
public:
	General(sint4 mapWidth, sint4 mapHeight);
	virtual ~General();
	void CreateGrid();		//Initializes the risk grid
	void Loop();			//The General's game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetEnemies(Vector<Unit> theEnemies);	//Set the list of enemies
	void Print();			//Prints a debugging view of the risk grid


	struct Tile
	{
		sint4 x;
		sint4 y;
		sint4 risk;
	};
private:
	real8 width;			//width of map
	real8 height;			//height of map
	sint4 xGrid;			//xSize of grid
	sint4 yGrid;			//ySize of grid
	Vector<Unit> enemies;	//List of enemies

	Tile ** grid;
};

#endif /* GENERAL_H_ */
