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

//constant values for marine and tanks used with unit.GetType()
const sint4 marine = 1;
const sint4 tank = 2;

real8 TILEWIDTH = 0;
real8 TILEHEIGHT = 0;

General::General(sint4 mapWidth, sint4 mapHeight)
{
	width = mapWidth;
	height = mapHeight;
	//default 10x10 risk grid
	xGrid = 10;
	yGrid = 10;
	SAFE_VALUE = 10;

	TILEWIDTH = width/xGrid;
	TILEHEIGHT = height/yGrid;

	CreateGrid();
}

General::~General()
{
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


		//std::cout << location.x << location.y << std::endl;
		real8 riskValue = 0;

		if (type == marine)
		{
			riskValue = 1*health;
		}
		else if (type == tank)
		{
			//if tank is sieged
			if (enemy.GetMode() == 2)
			{
				riskValue = 5*health;
			}
			else
			{
				riskValue = 3*health;
			}
		}
		Tile* tile = ConvertToGridTile(location);

		if (tile->risk < 50)
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

		//std::cout << location.x << location.y << std::endl;
		real8 riskValue = 0;

		if (type == marine)
		{
			riskValue = -1*health;
		}
		else if (type == tank)
		{
			//if tank is in siege mode
			if (friendly.GetMode() == 2)
			{
				riskValue = -5*health;
			}
			else
			{
				riskValue = -3*health;
			}
		}
		Tile* tile = ConvertToGridTile(location);

		if (tile->risk > -50)
		{
			tile->risk += riskValue;
		}
	}

	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			Tile* tile = &(grid[j][i]);
			if (tile->risk != 0)
				tile->risk *= 0.8;
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

vec2 General::ConvertToLocation(Tile& tile)
{
	//convert to appropriate location (center of tile)
	real8 x = ((real8)tile.x/(real8)xGrid)*width;
	real8 y = ((real8)tile.y/(real8)yGrid)*height;

	std::cout << x << "," << y << std::endl;

	vec2 location = vec2(x + (TILEWIDTH/2),y + (TILEHEIGHT/2));
	return location;
}

bool General::isLocationSafe(vec2 location)
{
	Tile* tile = ConvertToGridTile(location);

	if (tile->risk <= SAFE_VALUE)
	{
		return true;
	}
	return false;
}

vec2 General::GetFallBackLocation(vec2 location)
{
	Tile* currentTile = ConvertToGridTile(location);
	float minDist = 3;
	Tile* fallBack = currentTile;

	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			Tile* tile = &(grid[j][i]);
			if (tile->risk <= SAFE_VALUE)
			{
				float dist = tile->GetDistanceTo(*currentTile);
				if (dist < minDist)
				{
					minDist = dist;
					fallBack = tile;
				}
			}
		}
	}
	vec2 safeLocation = vec2(ConvertToLocation(*fallBack));
	//std::cout << safeLocation.x << "," << safeLocation.y << std::endl;
	return safeLocation;
}

vec2 General::GetClosestTarget(vec2 location)
{
	Tile* currentTile = ConvertToGridTile(location);
	float minDist = xGrid;
	Tile* closestTarget = currentTile;

	for (int i = 0; i < yGrid; ++i)
	{
		for (int j = 0; j < xGrid; ++j)
		{
			Tile* tile = &(grid[j][i]);
			if (tile->risk > SAFE_VALUE)
			{
				float dist = tile->GetDistanceTo(*currentTile);
				if (dist < minDist)
				{
					minDist = dist;
					closestTarget = tile;
				}
			}
		}
	}
	vec2 target = vec2(ConvertToLocation(*closestTarget));
	//std::cout << target.x << "," << target.y << std::endl;
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
