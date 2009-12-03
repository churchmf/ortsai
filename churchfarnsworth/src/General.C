/*
 * General.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "General.H"

#include "Application.H"
#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"
//////////////////////////////////////////////////////////////
////////////    CONSTANTS AND GAME VARIABLES      ///////////
/////////////////////////////////////////////////////////////
//constant values for marine and tanks used with unit.GetType()
const sint4 MARINE = 1;
const sint4 TANK = 2;

//default 10x10 risk grid
const sint4 xGrid = 10;
const sint4 yGrid = 10;

//cut off value to determine if a risk value is safe
const sint4 SAFE_VALUE = 3;

//deals with grid values
const sint4 MARINE_RISK = 1;
const sint4 TANK_RISK = 3;
const sint4 TANK_SIEGE_RISK = 5;
const sint4 MAX_RISK = 50;
const real8 RISK_DEPRECIATION = 0.8;
//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////

General::General(sint4 mapWidth, sint4 mapHeight)
{
	width = mapWidth;
	height = mapHeight;

	TILEWIDTH = width/xGrid;
	TILEHEIGHT = height/yGrid;

	CreateGrid();
}

General::~General()
{
	delete [] grid;
	delete grid;
}

void General::CreateGrid()
{
	grid = new Tile*[xGrid];
	for (int i = 0; i < xGrid; ++i)
	{
		grid[i] = new Tile[yGrid];
		for (int j = 0; j < yGrid; ++j)
		{
			grid[i][j].x = i;
			grid[i][j].y = j;
			grid[i][j].risk = 0;
		}
	}
}

void General::SetUnits(Vector<Unit> theEnemies,Vector<Unit> theUnits)
{
	enemies = theEnemies;
	myUnits = theUnits;
}

void General::Loop(Vector<Unit> theEnemies,Vector<Unit> theUnits)
{
	SetUnits(theEnemies,theUnits);

	//populate enemy values
	for(size_t i(0); i<enemies.size(); ++i)
	{
		Unit & enemy(enemies[i]);
		sint4 type = enemy.GetType();
		vec2 location = enemy.GetPosition();

		//enemy health %
		real8 hp = enemy.GetHitpoints();
		real8 maxHp = enemy.GetMaxHitpoints();
		real8 health = (hp/maxHp);

		real8 riskValue = 0;

		if (type == MARINE)
		{
			riskValue = MARINE_RISK*health;
		}
		else if (type == TANK)
		{
			//if tank is in siege mode
			if (enemy.GetMode() == 2)
			{
				riskValue = TANK_SIEGE_RISK*health;
			}
			else
			{
				riskValue = TANK_RISK*health;
			}
		}
		Tile* tile = ConvertToGridTile(location);

		if (tile->risk < MAX_RISK)
		{
			tile->risk += riskValue;
		}
	}

	//populate friendly values
	for(size_t i(0); i<myUnits.size(); ++i)
	{
		Unit & friendly(myUnits[i]);
		sint4 type = friendly.GetType();
		vec2 location = friendly.GetPosition();

		//enemy health %
		real8 hp = friendly.GetHitpoints();
		real8 maxHp = friendly.GetMaxHitpoints();
		real8 health = (hp/maxHp);

		real8 riskValue = 0;

		if (type == MARINE)
		{
			riskValue = -MARINE_RISK*health;
		}
		else if (type == TANK)
		{
			//if tank is in siege mode
			if (friendly.GetMode() == 2)
			{
				riskValue = -TANK_SIEGE_RISK*health;
			}
			else
			{
				riskValue = -TANK_RISK*health;
			}
		}
		Tile* tile = ConvertToGridTile(location);

		if (tile->risk > -MAX_RISK)
		{
			tile->risk += riskValue;
		}
	}

	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			Tile* tile = &(grid[j][i]);
			//round off weak risk tiles
			if (abs(tile->risk) < 1)
			{
				tile->risk = 0;
			}
			//depreciate the risk of the tile
			if (tile->risk != 0)
				tile->risk *= RISK_DEPRECIATION;

		}
	}
}

General::Tile* General::ConvertToGridTile(vec2 location)
{
	//convert to appropriate grid tile
	real8 x = location.x;
	real8 y = location.y;

	sint4 xLoc = xGrid * (x/width);
	sint4 yLoc = yGrid * (y/height);

	Tile* tile = &(grid[xLoc][yLoc]);
	return tile;
}

vec2 General::ConvertToLocation(Tile tile)
{
	//convert to appropriate location (center of tile)
	real8 x = ((real8)tile.x/(real8)xGrid)*width;
	real8 y = ((real8)tile.y/(real8)yGrid)*height;

	vec2 location = vec2((sint4)(x + (TILEWIDTH/2)),(sint4)(y + (TILEHEIGHT/2)));
	return location;
}

bool General::IsLocationSafe(vec2 location)
{
	Tile* tile = ConvertToGridTile(location);

	if (tile->risk <= SAFE_VALUE)
	{
		return true;
	}
	return false;
}

vec2 General::FindSafeWaypoint(vec2 current, vec2 goal)
{
	Tile* goalTile = ConvertToGridTile(goal);
	Tile* currentTile = ConvertToGridTile(current);
	real8 shortestDistance = currentTile->GetDistanceTo(*goalTile);
	Tile* shortestTile = currentTile;

	//check in surrounding tiles
	for(int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			//check tile boundries
			if ((currentTile->x+j > 0 && currentTile->x+j < xGrid) && (currentTile->y+i > 0 && currentTile->y+i < yGrid))
			{
				Tile* tile = &(grid[currentTile->x+j][currentTile->y+i]);
				real8 distance = tile->GetDistanceTo(*goalTile);
				if (tile->risk <= SAFE_VALUE && distance < shortestDistance)
				{
					shortestDistance = distance;
					shortestTile = tile;
				}
			}
		}
	}
	return ConvertToLocation(*shortestTile);
}

vec2 General::GetFallBackLocation(vec2 current, vec2 enemy)
{
	Tile* enemyTile = ConvertToGridTile(enemy);
	Tile* currentTile = ConvertToGridTile(current);
	real8 furthestDistance = currentTile->GetDistanceTo(*enemyTile);
	Tile* furthestTile = currentTile;

	//check in surrounding tiles
	for(int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			//check tile boundries
			if ((currentTile->x+j > 0 && currentTile->x+j < xGrid) && (currentTile->y+i > 0 && currentTile->y+i < yGrid))
			{
				Tile* tile = &(grid[currentTile->x+j][currentTile->y+i]);
				real8 distance = tile->GetDistanceTo(*enemyTile);
				if (tile->risk <= SAFE_VALUE && distance > furthestDistance)
				{
					furthestDistance = distance;
					furthestTile = tile;
				}
			}
		}
	}
	return ConvertToLocation(*furthestTile);
}

//NOTE: the results of this assessment change drastically with the size of the risk grid
//it is recommended to search a larger number of surrounding tiles as the grid gets denser to maintain desired functionality
//good rule of thumb is surrounding tiles should encompass range of enemy tanks
bool General::IsOutNumbered(vec2 location)
{
	Tile* centerTile = ConvertToGridTile(location);
	real8 currentTile = centerTile->risk;
	real8 surroundingTiles = 0;

	//quick check in local tile
	if (currentTile > SAFE_VALUE)
		return true;

	//check in surrounding tiles
	for(int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			//check tile boundries
			if ((centerTile->x+j > 0 && centerTile->x+j < xGrid) && (centerTile->y+i > 0 && centerTile->y+i < yGrid))
			{
				Tile* tile = &(grid[centerTile->x+j][centerTile->y+i]);
				surroundingTiles += tile->risk;
			}
		}
	}

	//if the surrounding risk values are higher than your current tile
	if (surroundingTiles > abs(currentTile))
		return true;
	return false;
}

vec2 General::FindEmptyWaypoint(vec2 location, vec2 target)
{
	//determine how crowded an area is based on its risk value
	//dangerous because it may see combat zones as potentially empty areas
	Tile* currentTile = ConvertToGridTile(location);
	Tile* targetTile = ConvertToGridTile(target);

	Tile* shortestTile = currentTile;
	real8 shortestDistance = currentTile->GetDistanceTo(*targetTile);

	//check in surrounding tiles
	for(int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			//check tile boundries
			if ((currentTile->x+j > 0 && currentTile->x+j < xGrid) && (currentTile->y+i > 0 && currentTile->y+i < yGrid))
			{
				Tile* tile = &(grid[currentTile->x+j][currentTile->y+i]);
				real8 distance = tile->GetDistanceTo(*targetTile);
				if ((tile->risk < SAFE_VALUE) && (tile->risk > -2*SAFE_VALUE) && (distance < shortestDistance))
				{
					shortestDistance = distance;
					shortestTile = tile;
				}
			}
		}
	}
	return ConvertToLocation(*shortestTile);
}

vec2 General::GetClosestTarget(vec2 location)
{
	Tile* currentTile = ConvertToGridTile(location);
	real8 minDist = 100;
	Tile* closestTarget = currentTile;

	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			Tile* tile = &(grid[j][i]);
			if (tile->risk > 0)
			{
				real8 dist = tile->GetDistanceTo(*currentTile);
				//std::cout << dist << std::endl;
				if (dist < minDist)
				{
					//std::cout << j << "," << i << std::endl;
					minDist = dist;
					closestTarget = tile;
				}
			}
		}
	}
	vec2 target = ConvertToLocation(*closestTarget);
	return target;
}

void General::Print()
{
	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			std::cout << "\t";
			sint4 risk = grid[j][i].risk;
			if (risk != 0)
				std::cout << "[" << risk << "]";
			else
				std::cout << "[ " << "]";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
