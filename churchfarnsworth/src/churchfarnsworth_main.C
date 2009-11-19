/*
 * churchfarnsworth_main.C
 *
 *  Created on: Nov 18, 2009
 *      Author: mfchurch
 *
 *
-A file called "src/[app]_main.C", where [app] is the name of the folder containing your "src/" directory.
For instance, in the case of "lab2.template", we need a "src/template_main.C", which will contain our program's "main()" function.
 */
#include "Application.H"
#include "Helper.H"
#include "Lieutenant.H"

#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"

class MyApplication : public Application
{
public:
	void OnReceivedView(GameStateModule & gameState);
};

//Seems to be the game loop, I think we should put out game logic in here - Matt
void MyApplication::OnReceivedView(GameStateModule & gameState)
{
	// INFO: You can examine the GameChanges class to determine many things
	// about what has changed between the last view and the current one
	// See GameStateModule::get_changes(),
	// GameChanges::new_tile_indexes, GameChanges::new_boundaries,
	// GameChanges::new_objs, GameChanges::changed_objs,
	// GameChanges::vanished_objs, and GameChanges::dead_objs

	// INFO: You can examine the Game class to determine everything about
	// the state of the game according to the current view
	const Game & game(gameState.get_game());

	// INFO: You can examine the Map<GameTile> class to determine the
	// topography of the game map. The map is composed of GameTiles, which
	// determine the type and traversability of the terrain. For now, we
	// will determine the maximum coordinates of a point on the map
	const Map<GameTile> & map(game.get_map());
	const sint4 maxCoordX(map.get_width()  * game.get_tile_points());
	const sint4 maxCoordY(map.get_height() * game.get_tile_points());

	// INFO: Clients are referenced by integer IDs, starting from zero
	// You can get your own client's ID with Game::get_client_player()
	// Game::get_player_num() will return the number of players in the game

	// Iterate through all teams and store the living units
	const sint4	myClient(game.get_client_player());
	Vector<Unit> myUnits,enemies;

	//predefined unit types, used with unit.GetType() - Matt
	const sint4 marine = 1;
	const sint4 tank = 2;

	//AQUIRE AND SORT ALL OBJECTS
	for(int team(0); team<game.get_player_num(); ++team)
	{
		// Get the units on this team
		const Game::ObjCont & units(game.get_objs(team));
		FORALL(units, it)
		{
			// Skip non-game objects (such as graphics objects)
			if(!(*it)->get_GameObj()) continue;

			// Skip dead units
			Unit unit(game,*it);
			if(!unit.IsAlive()) continue;

			// Store unit as our unit or as enemy
			if(team == myClient)
			{
				myUnits.push_back(unit);
			}
			else
			{
				enemies.push_back(unit);
			}
		}
	}

	//GAME LOOP
	for(size_t i(0); i<myUnits.size(); ++i)
	{
		Unit & unit(myUnits[i]);

		// If the unit has a weapon, look for targets
		if(unit.HasWeapon())
		{
			// Cache the unit's position and the range of its weapon
			const vec2	position(unit.GetPosition());
			const sint4 range(unit.GetWeaponRange());
			const sint4 rangeSq(range*range);
			DrawDebugCircle(position,range,Color(1,1,1));

			// Choose the first enemy unit we find in range

			for(size_t j(0); j<enemies.size(); ++j)
			{
				const Unit & enemy(enemies[j]);
				if(position.GetDistanceSqTo(enemy.GetPosition()) <= rangeSq)
				{
					unit.Attack(enemy);
					DrawDebugLine(position,enemy.GetPosition(),Color(1,1,1));
					break;
				}
			}
		}

		/*
		// If this unit is currenly not moving
		if(!unit.IsMoving())
		{
			unit.MoveTo(vec2(rand()%maxCoordX, rand()%maxCoordY), unit.GetMaxSpeed());
		}
		*/
		if(!unit.IsMoving())
		{
			if (unit.GetType() == marine)
				unit.MoveTo(vec2(0, 0), unit.GetMaxSpeed());
			if (unit.GetType() == tank)
				unit.MoveTo(vec2(maxCoordX, maxCoordY), unit.GetMaxSpeed());
		}
	}
}

int main(int argc, char * argv[])
{
	try
	{
		MyApplication app;
		return app.Run(argc, argv);
	}
	catch(...)
	{
		return -1;
	}
}