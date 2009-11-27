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

//////////////////////////////////////////////////////////////
////////////    CONSTANTS AND GAME VARIABLES      ///////////
/////////////////////////////////////////////////////////////
//constant values for marine and tanks used with unit.GetType()
const sint4 MARINE = 1;
const sint4 TANK = 2;

//maximum number of marines and tanks per lieutenant squad
const sint4 MAX_MARINES = 10;
const sint4 MAX_TANKS = 4;
//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////

class MyApplication : public Application
{
public:
	void OnReceivedView(GameStateModule & gameState, Movement::Context& mc);
	void Initialize(GameStateModule & gameState,  Movement::Context& mc);
private:
	Vector<Lieutenant*> Lieutenants;		//Represents a Vector of Lieutenants
	General* general;						//Represents a General
	Captain* captain;						//Represents a Captain

};

//INITIALIZATION (called in first frame of main game loop)
void MyApplication::Initialize(GameStateModule & gameState,  Movement::Context& mc)
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

	std::cout << "CREATE LIEUTS" << std::endl;
	for (int i=0;i<5;++i)
	{
		Lieutenant* lieutenant = new Lieutenant();
		Lieutenants.push_back(lieutenant);
	}

	std::cout << "ALLOCATE UNITS" << std::endl;
	uint4 m = 0;
	uint4 t = 0;
	for(size_t i(0); i< myUnits.size(); ++i)
	{
		Unit & unit(myUnits[i]);

		if(unit.HasWeapon())
		{
			if(unit.GetType() == MARINE)
			{
				if (Lieutenants[m]->MarineSize() >= MAX_MARINES
				&& m < Lieutenants.size())
					m++;
				Lieutenants[m]->AssignUnit(unit);
			}
			if(unit.GetType() == TANK)
			{
				if (Lieutenants[t]->TankSize() >= MAX_TANKS
				&& t < Lieutenants.size())
					t++;
				Lieutenants[t]->AssignUnit(unit);
			}
		}
	}

	//INITIALIZE GENERAL
	general = new General(maxCoordX, maxCoordY);

	//INITIALIZE CAPTAIN
	captain = new Captain(*general);
	captain->SetLieutenants(Lieutenants);

	//SETUP LIEUTENANT FORMATIONS
	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		Lieutenants[i]->Loop(mc,enemies);
		//Lieutenants[i]->MoveTo(vec2(20,maxCoordY/(i+1)));
		Lieutenants[i]->DoFormation(vec2(1,0));
	}
}


//MAIN GAME LOOP
void MyApplication::OnReceivedView(GameStateModule & gameState, Movement::Context& mc)
{
	//////////////////////////////////////////////////////////////
	///////////////////   INITIALIZATION      ///////////////////
	/////////////////////////////////////////////////////////////
	static bool firstFrame = true;
	if(firstFrame)
	{
		Initialize(gameState, mc);
		firstFrame = false;
	}
	//////////////////////////////////////////////////////////////
	/////////////////   END INITIALIZATION      /////////////////
	/////////////////////////////////////////////////////////////

	// INFO: You can examine the Game class to determine everything about
	// the state of the game according to the current view
	const Game & game(gameState.get_game());

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

	//////////////////////////////////////////////////////////////
	//////////////////    COMMANDER LOOPS      //////////////////
	/////////////////////////////////////////////////////////////

	general->Loop(enemies, myUnits);
	general->Print();

	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		Lieutenant* lieutenant(Lieutenants[i]);
		lieutenant->Loop(mc,enemies);
	}

	//////////////////////////////////////////////////////////////
	////////////////     END COMMANDER LOOPS      ///////////////
	/////////////////////////////////////////////////////////////

	bool draw_flag = true;
	/*
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
		}
	}
	*/

	// Lieutenant debugging circle
	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		vec2 ltPos = Lieutenants[i]->GetLocation();
		DrawDebugCircle(ltPos, 100, Color(1,1,0));
	}
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
