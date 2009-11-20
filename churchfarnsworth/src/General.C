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

	CreateGrid();
}

General::~General()
{

}

void General::CreateGrid()
{
	grid = new Tile*[width];
	for (int i = 0; i < width; ++i)
	{
		grid[i] = new Tile[height];
		for (int j = 0; j < height; ++j)
		{
			grid[i][j].x = i;
			grid[i][j].y = j;
			grid[i][j].risk = 0;
		}
	}
}

void General::SetEnemies(Vector<Unit> theEnemies)
{
	enemies = theEnemies;
}

void General::Loop()
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			grid[i][j].risk = 0;
		}
	}

	for(size_t i(0); i<enemies.size(); ++i)
	{
		Unit & enemy(enemies[i]);
		sint4 type = enemy.GetType();
		vec2 location = enemy.GetPosition();
		sint4 riskValue = 0;

		if (type == marine)
		{
			riskValue = 1;
		}
		else if (type == tank)
		{
			//if tank is sieged
			if (enemy.GetMode() == 2)
			{
				riskValue = 5;
			}
			else
			{
				riskValue = 3;
			}
		}
		Tile tile = grid[location.x][location.y];
		if (tile.risk > 0)
			tile.risk *= 0.9;

		if (tile.risk < 50)
			tile.risk += riskValue;
	}
}

void General::Print()
{
	for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				std::cout << "\t";
				sint4 risk = grid[i][j].risk;
				std::cout << "[" << risk << "]" << std::endl;
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
