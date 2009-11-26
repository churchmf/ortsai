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
#include "Captain.H"
#include "General.H"

#include "Movement.H"
#include "TerrainBase.H"
#include "PathfindTask.H"
#include "Game.H"
#include "GameObj.H"
#include "GameStateModule.H"
#include "GameChanges.H"
using namespace std;

class MyApplication : public Application
{
public:
	void OnReceivedView(GameStateModule & gameState,Movement::Module& mm, Movement::Context& mc);
	void Initialize(GameStateModule & gameState);
private:
	Vector<Lieutenant*> Lieutenants;
	General* general;
	Captain* captain;
};

//Initialization (called before game loop)
void MyApplication::Initialize(GameStateModule & gameState)
{
	const Game & game(gameState.get_game());
	const Map<GameTile> & map(game.get_map());
	const sint4 maxCoordX(map.get_width()  * game.get_tile_points());
	const sint4 maxCoordY(map.get_height() * game.get_tile_points());

	const sint4	myClient(game.get_client_player());
	Vector<Unit> myUnits,enemies;


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

	//Lieutenants = Vector<Lieutenant>(5);

	std::cout << "CREATE LIEUTS" << std::endl;
	for (int i=0;i<5;++i)
	{
		Lieutenant* lieutenant = new Lieutenant(gameState);
		Lieutenants.push_back(lieutenant);
	}

	std::cout << "ALLOCATE UNITS" << std::endl;
	for(size_t i(0); i< myUnits.size(); ++i)
	{
		Unit & unit(myUnits[i]);

		if(unit.HasWeapon())
		{
			Lieutenants[0]->AssignUnit(unit);

		}
	}
	general = new General(maxCoordX, maxCoordY);
	//captain();
}


//Seems to be the game loop, I think we should put out game logic in here - Matt
void MyApplication::OnReceivedView(GameStateModule & gameState,Movement::Module& mm, Movement::Context& mc)
{
	static bool firstFrame = true;
	if(firstFrame)
	{
		Initialize(gameState);
		firstFrame = false;
	}
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

	general->Loop(enemies, myUnits);
	general->Print();

	bool draw_flag = true;
	//GAME LOOP

	const sint4 me(gameState.get_game().get_client_player());
	FORALL(gameState.get_changes().new_objs, it)
	{
		// If we have found a unit that belongs to us
		GameObj * gob((*it)->get_GameObj());
		if(gob && *(gob->sod.owner) == me)
		{
			// And if it is a mobile unit
			if(gob->bp_name() == "marine" || gob->bp_name() == "tank")
			{
				// Tell them to move to a specific spot
				mc.moveUnit(gob, Movement::TouchPoint(Movement::Vec2D(rand()%maxCoordX, rand()%maxCoordY)));
			}
		}
	}

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
			//DrawDebugCircle(position,range,Color(1,1,1));

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


			//Lieutenants[0]->DoFormation(vec2(1,0), gameState);
			// If this unit is currently not moving
			if(!unit.IsMoving())
			{
				//unit.MoveTo(vec2(rand()%maxCoordX, rand()%maxCoordY), unit.GetMaxSpeed());
			}
		}

		//mc.moveUnit(someUnit, Movement::MoveToPoint(x,y));
		/*
		if(!unit.IsMoving())
		{
			if (unit.GetType() == marine)
				unit.MoveTo(vec2(0, 0), unit.GetMaxSpeed());
			if (unit.GetType() == tank)
				unit.MoveTo(vec2(maxCoordX, maxCoordY), unit.GetMaxSpeed());
		}
		*/
	}

	// tester stuff
	vec2 ltPos = Lieutenants[0]->GetLocation();
	DrawDebugCircle(ltPos, 100, Color(1,1,0));
	draw_flag = false;
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
