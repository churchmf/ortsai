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
	struct Tile
	{
		sint4 x;
		sint4 y;
		real8 risk;
	};

	General(sint4 mapWidth, sint4 mapHeight);
	virtual ~General();
	void CreateGrid();												//Initializes the risk grid
	void Loop(Vector<Unit> theEnemies,Vector<Unit> theUnits);		//The General's game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetUnits(Vector<Unit> theEnemies, Vector<Unit> theUnits);	//Set the list of enemies and friendly units
	void Print();													//Prints a debugging view of the risk grid
	bool isLocationSafe(vec2 location);								//Returns true if risk value is less than safe value
	Tile* ConvertToGridTile(vec2 location);							//Converts the given location on the map to the appropriate tile in the grid
	vec2 GetClosestTarget(vec2 location);							//Returns location of the closest cluster of enemies to the target location

private:
	real8 width;			//width of map
	real8 height;			//height of map
	sint4 xGrid;			//xSize of grid
	sint4 yGrid;			//ySize of grid
	Vector<Unit> enemies;	//List of enemies
	Vector<Unit> myUnits;	//List of the friendly Units
	real8 safeValue;		//cut off value to determine if a risk value is safe

	Tile ** grid;
};

#endif /* GENERAL_H_ */
