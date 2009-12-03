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
		Tile() {}
		Tile(sint4 x, sint4 y) : x(x), y(y) {}
		float GetLength() const { return sqrtf(static_cast<float>(x*x+y*y)); }
		float GetDistanceTo(const Tile & t) const { return (Tile(x-t.x,y-t.y)).GetLength(); }
		sint4 x;
		sint4 y;
		real8 risk;
	};

	//General* operator = (General&) const { return *this; }

	General(sint4 mapWidth, sint4 mapHeight);
	virtual ~General();
	void CreateGrid();												//Initializes the risk grid
	void Loop(Vector<Unit> theEnemies,Vector<Unit> theUnits);		//The General's game loop, all logic should be done here. Is called once every iteration of the game loop.
	void SetUnits(Vector<Unit> theEnemies, Vector<Unit> theUnits);	//Set the list of enemies and friendly units
	void Print();													//Prints a debugging view of the risk grid

	bool IsLocationSafe(vec2 location);								//Returns true if risk value is less than safe value
	bool IsOutNumbered(vec2 location);								//Returns true if the given location is potentially OutNumbered
	vec2 FindSafeWaypoint(vec2 current, vec2 goal);					//Returns a neighbouring safe tile closer than the current Tile, if none exists it returns the current tile. *NOTE* this is not pathfinding.
	vec2 FindEmptyWaypoint(vec2 location, vec2 target);				//Returns a neighbouring empty tile closer than the current Tile, if none exists it returns the current tile. *NOTE* this can be dangerous to use

	vec2 GetClosestTarget(vec2 location);							//Returns tile location of nearest cluster of enemies to the provided location
	vec2 GetStrongestTarget();										//Returns the strongest location in the enemy's formation
	vec2 GetWeakestTarget();										//Returns the weakest location in the enemy's formation
	vec2 GetFallBackLocation(vec2 location, vec2 enemy);			//Returns tile location of furthest neighbouring tile from enemy

	Tile* ConvertToGridTile(vec2 location);							//Converts the given location on the map to the appropriate tile in the grid
	vec2 ConvertToLocation(Tile tile);								//Converts the given tile to the center location of the tile on the map

	real8 width;			//width of map
	real8 height;			//height of map

private:
	Vector<Unit> enemies;	//List of enemies
	Vector<Unit> myUnits;	//List of the friendly Units

	//width and height of each tile in the grid(initialized in constructor)
	real8 TILEWIDTH;
	real8 TILEHEIGHT;

	Tile ** grid;
};

#endif /* GENERAL_H_ */
