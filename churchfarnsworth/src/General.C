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

General::General(sint4 mapWidth, sint4 mapHeight)
{
	width = mapWidth;
	height = mapHeight;
	//default 10x10 risk grid
	xGrid = 10;
	yGrid = 10;
	safeValue = 3;

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

		if (tile->risk < 50 || tile->rish >-50)
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
			riskValue = -2*health;
		}
		else if (type == tank)
		{
			//if tank is sieged
			if (friendly.GetMode() == 2)
			{
				riskValue = -8*health;
			}
			else
			{
				riskValue = -4*health;
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
	//std::cout << (hp/maxHp) << std::endl;
	sint4 xLoc = xGrid * (x/width);
	sint4 yLoc = yGrid * (y/height);

	Tile* tile = &(grid[xLoc][yLoc]);
	//std::cout << xLoc << yLoc << std::endl;
	return tile;
}

bool General::isLocationSafe(vec2 location)
{
	Tile* tile = ConvertToGridTile(location);

	if (tile->risk < safeValue)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//needs to be finished
vec2 General::GetClosestTarget(vec2 location)
{
	Tile* tile = ConvertToGridTile(location);
	sint4 xLoc = tile->x;
	sint4 yLoc = tile->y;

	for (int i = yLoc-1; i < yLoc+1; ++i)
	{
		for (int j = xLoc-1; j < xLoc+1; ++j)
		{
			sint4 x = (i/xGrid)*width;
			sint4 y = (j/yGrid)*height;
			//vec2 tileLoc = vec2(x,y);

			if (grid[i][j].risk > safeValue)
			{
				//float diff = location.GetDistanceTo(tileLoc);
				return vec2(x*width,y*height);
			}
		}
	}
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

//General Loop
/*
 * for each tile in grid:
  if tile contains marine:
    if friendly:
      tile.risk -= 1
    else:
      tile.risk += 1
  if tile contains tank:
    if friendly:
      if deployed:
        tile.risk -= 5
      else:
        tile.risk -= 3
    else:
      if deployed:
        tile.risk += 5
      else:
        tile.risk += 3
depreciate tile value
 */
