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

#include <algorithm>
using namespace std;

//////////////////////////////////////////////////////////////
////////////    CONSTANTS AND GAME VARIABLES      ///////////
/////////////////////////////////////////////////////////////
//Number of Lieutenants, Captain will dynamically know this based on the size of it's Vector
const sint4 NUM_LIEUTENANTS = 5;
//////////////////////////////////////////////////////////////
//////////    END CONSTANTS AND GAME VARIABLES      /////////
/////////////////////////////////////////////////////////////

//TO IMPLEMENT USER STRATEGY SEE CAPTAIN LOOP FOR HIGH LEVEL BEHAVIOURS
//TO IMPLEMENT USER STRATEGY SEE LIEUTENANT LOOP FOR LOW LEVEL BEHAVIOURS
class MyApplication : public Application
{
public:
	void OnReceivedView(GameStateModule & gameState, Movement::Context& mc);
	void Initialize(GameStateModule & gameState,  Movement::Context& mc);

	struct UnitCompare
	{
	  bool operator() (Unit i,Unit j) { return (i.GetPosition().y < j.GetPosition().y);}
	} compUnits;

private:
	Vector<Lieutenant*> Lieutenants;		//Represents a Vector of Lieutenants
	General* general;						//Represents a General
	Captain* captain;						//Represents a Captain

};

//////////////////////////////////////////////////////////////
///////////////////   INITIALIZATION      ///////////////////
/////////////////////////////////////////////////////////////
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

	//CREATE LIEUTENANTS
	for (int i=0;i<NUM_LIEUTENANTS;++i)
	{
		Lieutenant* lieutenant = new Lieutenant();
		Lieutenants.push_back(lieutenant);
	}

	//Pass initial Movement Context into Lieutenants
	for(size_t i(0); i< Lieutenants.size(); ++i)
	{
		Lieutenants[i]->Loop(mc,enemies);
		Lieutenants[i]->SetEnemies(enemies);
	}

	//sort units by y positions, good idea for initial formations
	sort(myUnits.begin(), myUnits.end(), compUnits);

	//INITIALIZE GENERAL
	general = new General(maxCoordX, maxCoordY);

	//INITIALIZE CAPTAIN
	captain = new Captain(*general);
	captain->SetLieutenants(Lieutenants);
	captain->DistributeUnits(myUnits);

	//INITIAL FORMATION
	captain->Deploy();
}
//////////////////////////////////////////////////////////////
/////////////////   END INITIALIZATION      /////////////////
/////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
///////////////////      GAME LOOP       // /////////////////
/////////////////////////////////////////////////////////////
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

	//////////////////////////////////////////////////////////////
	///////////////////    GAME UPDATES      // /////////////////
	/////////////////////////////////////////////////////////////
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
	//////////////////   END GAME UPDATES     ///////////////////
	/////////////////////////////////////////////////////////////

	//check if there are any units remaining
	if (!enemies.empty() && !myUnits.empty())
	{
		//////////////////////////////////////////////////////////////
		//////////////////    COMMANDER LOOPS      //////////////////
		/////////////////////////////////////////////////////////////

		general->Loop(enemies, myUnits);
		//general->Print();	//DEBUGGING PRINT FOR GENERAL: USE TO VIEW GRID

		captain->Loop(game.get_view_frame());

		for(size_t i(0); i< Lieutenants.size(); ++i)
		{
			Lieutenants[i]->Loop(mc,enemies);
		}

		//////////////////////////////////////////////////////////////
		////////////////     END COMMANDER LOOPS      ///////////////
		/////////////////////////////////////////////////////////////

		//DEBUGGING CIRCLES
		bool draw_flag = true;
		for(size_t i(0); i< Lieutenants.size(); ++i)
		{
			vec2 ltPos = Lieutenants[i]->GetCurrentPosition();
			if (ltPos.x > 0 && ltPos.y > 0)
				DrawDebugCircle(ltPos, 160, Color(1,1,0));
		}
		draw_flag = false;
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
