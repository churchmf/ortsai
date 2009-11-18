// $Id: SampleEventHandler.C 5301 2007-06-25 19:36:28Z lanctot $

// This is an ORTS file (c) Michael Buro, Andrew Downing, licensed under the GPL 3 or any later version

// A sample client for ORTS that shows how to access client data and play game 4
// most of Andrew's stuff is after the comment saying "- setting object actions"

// HIGH-LEVEL ALGORITHM:
//
// My client picks to use either line mode or siege mode.
// The chances of it picking each strategy is based on its win rates, which is saved in a data file.
//
// In line mode, units form 2 lines based on their range and slowly approach the opponent.
// Units try to stay roughly the same distance from their neighbor and get in range of the opponent at the same time.
// When close enough, tanks switch into siege mode.
// Marines often move out of range when reloading, but will try to get as close as they can to tanks in siege mode.
// Marines try to stay out of siege tank range unless opponent marines can shoot at our siege tanks.
// If units are on average too far away from the opponent, they will re-form lines and approach again.
//
// In corner mode, the tanks move to the corner and go into siege mode with the marines surrounding them.
// When opponent tanks switch into siege mode, my tanks get first shot at them and
// marines charge at them so that the marines cannot get picked off by opponent siege tanks.
// If the opponent is still not nearby after 700 frames, my units switch into line mode.

// todo/questions (most of which were originally in the sample AI file and don't apply):
//
// see comments beginning with "todo:" and maybe "todo?:"
//
// game scripts:
// - what to do with workers during build time
// - similarly, how to define mining?
// - how is attack range defined (distance between centers/hulls)?
//
// client:
// - how to access global player data?
// - how to compute the set of visible tiles?
// - how to compute tiles visible from a unit?
// - how to decide what can be attacked by a specific unit?

// FORS/FORALL/ERR macros are defined in orts3/libs/kernel/src/Global.H

#define LINUX_MODE // uncomment this if on Linux
#include "SampleEventHandler.H"
#include "GameStateModule.H"
#include "GfxModule.H"
#include "Game.H"
#include "GameChanges.H"
#include "GameTile.H"
#include "ServerObjData.H"
#include "Options.H"
#include "GUI.H"
#include "DrawOnTerrain2D.H"
#ifndef LINUX_MODE
#  include "io.h"
#endif
#include "fcntl.h"
#include <cerrno> // for errno
#include <cstdio> // for perror()

using namespace std;

sint4 loaddatafile();
void savedatafile();
sint4 lockdatafile();
inline sint4 lineatdist(const sintptr *prange, real8 idealdist, real8 currentdist, sint4 currentpos, sint4 followpos);

const sint4 usefile = 1; // todo?: make locking work in Windows
#ifdef LINUX_MODE
const string lockpath = "game-data/andrewd_game4.lck";
const string datapath = "game-data/andrewd_game4.txt";
#else
const string lockpath = "game-data\\andrewd_game4.lck";
const string datapath = "game-data\\andrewd_game4.txt";
#endif
const sint4 nstrategy = 2; // # of strategies
const sint4 linest = 0;
const sint4 cornerst = 1;
const sint4 linenosiegest = 100;
sint4 strategy = 0;
sint4 firststrategy = 0; // sometimes changes out of corner mode if enemy isn't coming
sint4 totalwins[nstrategy]; // for learning algorithm
sint4 totalgames[nstrategy];
sint4 gameended = -1; // -1 = not over, 0 = lose, 1 = win, 2 = done saving
const uint4 unitcountest = 200; // todo?: "redim" array
//sint4 allswitch = 0;
sint4 maxrange; // biggest range that anyone might have
sint4 minrange = 0; // smallest range that anyone might have
long bestrangeavg = 1; // to help synchronize best range
long bestrangesum; // to help find bestrangeavg
//sint4 bestrangemin; // slow down the unit in the front
//sint4 bestrangeminnew;
sint4 lastbestid[unitcountest];
sint4 lastx[unitcountest];
sint4 lasty[unitcountest];
real8 enemyspdavg;
real8 idealdistapproach; // best distance w/o factoring range for everyone to be from opponent while approaching
sint4 approachcount = 0; // 0 if this is 1st approach, 1 if it is not
const sint4 unittypediv = 3; // divide radius by this to get unit type id
const uint4 unittypeest = 5;
// linepos2unit and unit2linepos are only calculated at the beginning
sint4 linepos2unit[unitcountest][unittypeest]; // units will try to form a line with this order
sint4 unit2linepos[unitcountest]; // 2nd coordinate is for unit type
const uint4 colarraysize = 150;
/*sint4 coltile[colarraysize][colarraysize];
uint4 coltiletime[colarraysize][colarraysize]; // last time tiles were updated (tiles aren't erased)
uint4 coltilesize;*/
sint4 unittypecount[unittypeest]; // of our units, this is only calculated on 1st frame
sint4 firstframe = 1;

// tile types from core.bp
// fixme: game script reflection would help here
// in the competition we won't use water tiles
// TILE_CLIFF: non-traversable by ground units

enum { TILE_UNKNOWN=0, TILE_WATER=1, TILE_GROUND=2, TILE_CLIFF=3 };

bool SampleEventHandler::handle_event(const Event& e)
{
  if (e.get_who() == GameStateModule::FROM) {

    //cout << "Event: " << e.who() << " " << e.what() << endl;

    if (e.get_what() == GameStateModule::STOP_MSG) {
      state.quit = true; return true;
    }

    if (e.get_what() == GameStateModule::READ_ERROR_MSG) {
      state.quit = state.error = true; return true;
    }

    if (e.get_what() == GameStateModule::VIEW_MSG) {

      compute_actions();
      state.gsm->send_actions();

      Game &game = state.gsm->get_game();

      // gfx and frame statistics

      if (state.gfxm) state.gfxm->process_changes();

      sint4 vf = game.get_view_frame();
      sint4 af = game.get_action_frame();
      sint4 sa = game.get_skipped_actions();
      sint4 ma = game.get_merged_actions();

      if ((vf % 20) == 0) cout << "[frame " << vf << "]" << endl;

      if (vf != af || sa || ma > 1) {

        cout << "frame " << vf;
        if (af < 0) cout << " [no action]";
        else if (vf != af) cout << " [behind by " << (vf - af) << " frame(s)]";
        if (sa) cout << " [skipped " << sa << "]";
        if (ma > 1) cout << " [merged " << ma << "]";
        cout << endl;
      }

	  if (gameended != -1) savedatafile();

      if (state.gfxm) {

        // don't draw if we are far behind
        if (abs(vf-af) < 10) {
          state.gfxm->draw();
          state.just_drew = true;
        }
      }

      if (state.gui) {
        state.gui->event();
        //        state.gui->display();
        if (state.gui->quit) exit(0);
      }

      // as a hack for flickering, gui->display() is called in dot.start()
      // so call everything that draws to the debug window between start() and end()
      DrawOnTerrain2D dot(state.gui);
      dot.start();
      dot.draw_line(-60, 10, -20, 10, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-60, 10, -65, 15, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-44, 10, -56, 40, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-28, 10, -36, 40, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-36, 40, -30, 34, Vec3<real4>(1.0, 1.0, 1.0));

      dot.draw_circle(-40, 25, 35, Vec3<real4>(1.0, 0.0, 1.0));

      dot.end();

      return true;
    }
  }

  return false;
}


void SampleEventHandler::compute_actions()
{
  const GameChanges &changes = state.gsm->get_changes();

  bool cinfo;
  Options::get("-cinfo", cinfo);

  if (cinfo) {

    // - what changed in the last simulation tick

    if (!changes.new_tile_indexes.empty())
      cout << "#new tiles = " << changes.new_tile_indexes.size() << endl;

    if (!changes.new_boundaries.empty())
      cout << "#new boundaries = " << changes.new_boundaries.size() << endl;

    // fixme: what about removed tiles and removed boundaries?
    // currently tiles and boundaries are never removed

    if (!changes.new_objs.empty())
      cout << "#new objects = " << changes.new_objs.size() << endl;

    if (!changes.changed_objs.empty())
      cout << "#changed objects = " << changes.changed_objs.size() << endl;

    if (!changes.vanished_objs.empty())
      cout << "#vanished objects = " << changes.vanished_objs.size() << endl;

    if (!changes.dead_objs.empty())
      cout << "#dead objects = " << changes.dead_objs.size() << endl;

  }

  const Game &game = state.gsm->get_game();
  const sint4 cid = game.get_client_player();

  // ***************** terrain ******************

  {
    const Map<GameTile> &map = game.get_map();

    bool minfo;
    Options::get("-minfo", minfo);

    if (minfo) {

      // - map dimensions

      cout << "Map width: = " << map.get_width()  << " tiles" << endl;
      cout << "Map height: " << map.get_height() << " tiles" << endl;

      // height field scale: HEIGHT_MULT = one tile width up, constant for now
      cout << "height mult: " << GameConst::HEIGHT_MULT << endl;

      // - new/updated tile indexes

      cout << "# of new/updated tiles " << changes.new_tile_indexes.size() << endl;
      cout << "Index(es): ";
      FORALL (changes.new_tile_indexes, it) cout << *it << " ";
      cout << endl;

      // - new boundaries

      const Vector<ScriptObj::WPtr> &boundaries = changes.new_boundaries;

      FORALL (boundaries, it) {

        const GameObj * g = (*it)->get_GameObj();
        if (g == 0) continue;

        cout << "new boundary (" << *g->sod.x1 << "," << *g->sod.y1 << ")-("
             << *g->sod.x2 << "," << *g->sod.y2 << ")"
             << endl;
      }

      // - all boundaries

      const Game::ObjCont &all_boundaries = game.get_boundaries();
      cout << "total # of boundaries = " << all_boundaries.size() << endl;
    }


    bool tinfo;
    Options::get("-tinfo", tinfo);

    if (tinfo) {

      // - all tiles

      FORS (y, map.get_height()) {
        FORS (x, map.get_width()) {
          const GameTile &tile = map(x,y);
          cout << "[" << x << "," << y << "]: ";

          sint4 hNW, hNE, hSE, hSW;
          Tile::Split split;

          // get corner heights and split type
          tile.get_topo(hNW, hNE, hSE, hSW, split);

          // tile types: see enum on top
          // split types: see orts3/libs/kernel/src/Tile.H
          // if split == Tile::NO_SPLIT => typeW is the tile type

          cout << "split:" << split
               << " type w:" << tile.get_typeW()
               << " type e:" << tile.get_typeE()
               << " heights:" << hNW << " " << hNE << " " << hSE << " " << hSW
               << endl;
        }

        cout << endl;
      }

      cout << endl;
    }


    // - currently visible tiles

    // ... to be implemented
  }

  // ***************** objects ******************

  bool oinfo;
  Options::get("-oinfo", oinfo);

  if (oinfo) {

    // units under client player control

    const Game::ObjCont &objs = game.get_objs(cid);

    FORALL (objs, it) {

      const GameObj * gob = (*it)->get_GameObj();
      if (!gob) ERR("not a game object!?");

      const ServerObjData &sod = gob->sod;

      // - fundamental properties

      // blueprint name ~ type name
      cout << "bp: " << gob->bp_name();

      if (!sod.in_game) { cout << " no gameobj" << endl; continue; }

      if (*sod.x < 0) { cout << "not on playfield" << endl; continue; }

      // object on playfield

      // sod.shape and sod.zcat values defined in orts3/libs/kernel/src/Object.H

      cout << "zcat: " << *sod.zcat
           << " max-speed: " << *sod.max_speed
           << " speed: " << *sod.speed
           << " moving: " << *sod.is_moving
           << " sight: " << *sod.sight;

      if (*sod.shape == Object::CIRCLE) {
        cout << " CIRCLE loc: (" << *sod.x << "," << *sod.y << ")"
             << " r: "<< *sod.radius;

      } else if (*sod.shape == Object::RECTANGLE) {
        cout << " RECT dim: (" << *sod.x1 << "," << *sod.y1 << ")-("
             << *sod.x2 << "," << *sod.y2 << ")";
      } else {
        cout << " UNKNOWN SHAPE";
      }

      // - other object properties

      // heading: 0 = stopped, 1...GAME_CONST::HEADING_N
      const sintptr * hd = gob->get_int_ptr("heading");
      if (hd) cout << " hd: " << GameConst::angle_from_dir(*hd, GameConst::HEADING_N);

      // ... more examples

      cout << endl;
    }
  }

  // - what an object currently sees

  // ... to be implemented

  // - setting object actions

  // *********************************
  // MOST OF ANDREW'S CODE STARTS HERE
  // *********************************

  // INDEX OF MAJOR CODE SECTIONS:
  //
  // SET VARIABLES THAT WILL STAY THE SAME FOR THE REST OF THE GAME
  //   MAKE LINK LIST THAT SORTS OUR UNITS BY THEIR Y VALUE
  //   CONVERT LINK LIST TO EASY-TO-USE ARRAY AND SEPARATE TYPES OF UNITS
  // PER-FRAME INITIALIZATION
  // MAIN FOR LOOP TO COMPUTE UNIT ACTIONS
  //   CHECK FOR TARGETS AND UNITS TO FOLLOW
  //   ATTACK TARGET
  //   MOVE IF NOT SHOOTING
  //     FIND BEST POSITION FOR LINE MODE
  //     FIND BEST POSITION FOR CORNER MODE
  //     STAY ACCEPTABLE DISTANCE FROM NEIGHBOR CLOSER TO THE CENTER
  //     ALTERNATE WAY TO FORM LINE
  //     DISCOURAGE GOING OFF MAP EDGE
  //     COLLISION DETECTION AND RESPONSE

  const Map<GameTile> &map = game.get_map();
  const Game::ObjCont &objs = game.get_objs(cid);
  const sint4 points_x = map.get_width()  * game.get_tile_points();
  const sint4 points_y = map.get_height() * game.get_tile_points();
  const sint4 n = game.get_player_num();
  const sint4 game_tick = game.tick();
  const uint4 switchtime = 230;
  sint4 unitcount = 0; // of our units
  sint4 enemyunitcount = 0;
  sint4 enemyunittypecount[unittypeest];
  sint4 enemyinrange[unitcountest]; // unit type that enemy unit is in range of
  sint4 bestunitcount[unitcountest]; // # of units following each enemy unit
  sint4 besttargethp[unitcountest]; // hp to be taken from each enemy unit
  // initialize/reset variables
  //bestrangeminnew = points_x + points_y;
  bestrangesum = 0;
  enemyspdavg = 0;
  FORS (i, unitcountest) {
    // test if units did more or less damage than predicted
    // prediction is based on minimum damage so more damage is ok
    /*ScriptObj * lbunittemp = game.get_cplayer_info().get_obj(i);
	if (lbunittemp && lbunittemp->get_GameObj() && besttargethp[i] > 0 && lasthp[i] > 0 && lasthp[i] - besttargethp[i] >= 0) {
	  GameObj * gob2 = lbunittemp->get_GameObj();
	  if (lasthp[i] - besttargethp[i] != *gob2->get_int_ptr("hp")) {
	    cout << "old-new=" << lasthp[i] - besttargethp[i] - *gob2->get_int_ptr("hp") << endl;
	  }
	}*/
    bestunitcount[i] = 0;
	besttargethp[i] = 0;
  }
  // **************************************************************
  // SET VARIABLES THAT WILL STAY THE SAME FOR THE REST OF THE GAME
  // **************************************************************
  if (1 == firstframe) {
	GameObj * firstunit; // unit with lowest y value
    GameObj * unitlinklist[unitcountest]; // link to unit with next-highest y pos
	if (usefile > 0) {
	  // preset variables in case cannot open data file
	  FORS (i, nstrategy) {
	    totalwins[i] = 1;
		totalgames[i] = 1;
	  }
#ifdef LINUX_MODE
      chdir(getenv("HOME"));
#endif
	  sint4 safetoopen = lockdatafile(); // check lock file in case another process is using it
	  if (safetoopen >= 0) {
		if (0 == loaddatafile()) { // get strategy info from data file
		  perror("REMARK: error opening data file, choosing random strategy");
		}
	  }
	  else {
	    perror("REMARK: error opening lock file, choosing random strategy");
	  }
#ifdef LINUX_MODE // todo: line below gets permission denied error on Windows
	  if (std::remove(lockpath.c_str()) != 0) perror("REMARK: cannot delete lock file");
#endif
	  // decide which strategy to use based on how successful they were
	  real8 winpercent[nstrategy];
	  real8 winpercenttotal = 0; // sum of all winpercents
	  real8 winpercentcount = 0; // to check which strategy the random number chose
	  uint4 randnum;
	  // set chances of each strategy being picked based on win percent
	  FORS (i, nstrategy ) {
	    winpercent[i] = totalwins[i] / real8(totalgames[i]) * 100;
		winpercenttotal = winpercenttotal + winpercent[i];
	  }
	  // randomly select a number below total of win percents
	  randnum = rand.rand_uint4() % uint4(winpercenttotal);
	  // choose strategy based on win percents and number picked above
	  FORS (i, nstrategy) {
	    winpercentcount = winpercentcount + winpercent[i];
		if (randnum <= winpercentcount) {
		  strategy = i; // choose strategy
		  break;
		}
	  }
	  cout << "strategy " << strategy << endl;
	}
	// sometimes corner mode switches to line mode so keep track of original strategy
	firststrategy = strategy;
	// initialize best distance w/o factoring range for everyone to be from opponent while approaching
	if (strategy != cornerst) { // corner strategy doesn't approach like that
      idealdistapproach = points_x * 0.6; // width of map * 0.6
	}
	minrange = points_x * 2; // initialize to a range guaranteed to be too large
    FORALL (objs, it) { // for each of our units
      GameObj * gob = (*it)->get_GameObj();
      if (gob && gob->sod.in_game && *gob->sod.x >= 0 && !gob->is_dead()) {
	    unitcount = unitcount + 1; // keep track of # of units
		/*if (*gob->sod.radius > coltilesize) {
		  coltilesize = *gob->sod.radius; // width/length of collision tiles is radius of biggest unit
		}*/
		// keep track of max & min range that anyone might have
		if (gob->component("weapon")) {
		  // if unit has bigger range than current max range then reset max range
		  if (*gob->component("weapon")->get_int_ptr("max_ground_range") > maxrange) {
			maxrange = *gob->component("weapon")->get_int_ptr("max_ground_range");
		  }
		  if (gob->component("weapon2")) { // tanks in siege mode have large range
		    maxrange = *gob->component("weapon2")->get_int_ptr("max_ground_range");
		  }
		  // if unit has smaller range than current min range then reset min range
		  if (*gob->component("weapon")->get_int_ptr("max_ground_range") < minrange) {
			minrange = *gob->component("weapon")->get_int_ptr("max_ground_range");
		  }
		}
		// ****************************************************
		// MAKE LINK LIST THAT SORTS OUR UNITS BY THEIR Y VALUE
		// ****************************************************
		const uint4 gobid = game.get_cplayer_info().get_id(gob);
		if (1 == unitcount) { // be first unit if there is none
		  firstunit = gob;
		}
		else if (*gob->sod.y < *firstunit->sod.y) { // replace first unit if y is lower
		  unitlinklist[gobid] = firstunit;
          firstunit = gob;
		}
		else { // we're not at the beginning of the list
		  sint4 linkid = game.get_cplayer_info().get_id(firstunit);
		  sint4 linkcount = 1;
		  while (linkid >= 0) {
			linkcount = linkcount + 1;
		    if (unitcount > linkcount) {
		      if (*gob->sod.y < *unitlinklist[linkid]->sod.y) {
				// we're between 2 units on the list
			    unitlinklist[gobid] = unitlinklist[linkid];
				unitlinklist[linkid] = gob;
				linkid = -100; // tell loop to exit
			  }
			  else {
			    linkid = game.get_cplayer_info().get_id(unitlinklist[linkid]); // continue along the list
			  }
			}
			else {
			  // we're at the end of the list
			  unitlinklist[linkid] = gob;
			  linkid = -100; // tell loop to exit
			}
		  }
		}
	  }
	}
	// ******************************************************************
	// CONVERT LINK LIST TO EASY-TO-USE ARRAY AND SEPARATE TYPES OF UNITS
	// ******************************************************************
	sint4 linkid = game.get_cplayer_info().get_id(firstunit);
	sint4 linepos;
	sint4 unittype = *firstunit->sod.radius / unittypediv;
	FORS (i, unittypeest) {
	  unittypecount[i] = 0; // reset unittypecount
	}
    for (linepos = 1; linepos < unitcount; linepos++) {
	  linepos2unit[unittypecount[unittype]][unittype] = linkid; // map line position and unit type to unit
	  unit2linepos[linkid] = unittypecount[unittype]; // map unit to line position
	  unittypecount[unittype] = unittypecount[unittype] + 1; // total count of each type of unit
	  unittype = *unitlinklist[linkid]->sod.radius / unittypediv; // compute unit type (e.g., marine = 1, tank = 2)
	  linkid = game.get_cplayer_info().get_id(unitlinklist[linkid]); // continue along the link list
	}
  }
  else {
    // ************************
    // PER-FRAME INITIALIZATION
    // ************************
    FORS (i, unittypeest) {
	  enemyunittypecount[i] = 0;
	  enemyinrange[i] = 0;
	}
    // find ideal distance to be from enemy
	// if not in corner mode and
	//   too far from the opponent (i.e. greater than 1.4 times approximate average range)
	//   then re-approach
	// if in corner mode and opponent hasn't approached after a while
	//   then switch to line mode and approach
	if ((strategy != cornerst && bestrangeavg - idealdistapproach >
		((minrange * unittypecount[1] + maxrange * unittypecount[2]) /
		(unittypecount[1] + unittypecount[2]) + maxrange - minrange) * 1.4)
		|| (cornerst == strategy && game_tick > 700 && bestrangeavg > maxrange * 1.5)) {
	  idealdistapproach = bestrangeavg - minrange; // approach again if we are far away
	  approachcount = 1; // use different avoidance code since order calculated earlier is out-of-date
	  strategy = linest; // change strategy if enemy isn't coming
	}
    // consider the speed they come towards us for idealdistapproach
	FORS (i, n) { // loop through players to count enemy units
	  const Game::ObjCont &client_objs = game.get_objs(i);
      FORALL (client_objs, it) {
        GameObj * gob = (*it)->get_GameObj(); // my unit
        if (gob && gob->sod.in_game && *gob->sod.x >= 0 && !gob->is_dead()) {
		  if (i == cid) {
	        unitcount = unitcount + 1;
	        const uint4 gobid = game.get_cplayer_info().get_id(gob);
	        ScriptObj * lbunittemp = game.get_cplayer_info().get_obj(lastbestid[gobid]);
		    if (lbunittemp && lbunittemp->get_GameObj()) { // if opponent unit hasn't died
		      GameObj * gob2 = lbunittemp->get_GameObj(); // opponent unit that my unit is following
		      real8 distsqrd = square(*gob2->sod.x - lastx[gobid]) + square(*gob2->sod.y - lasty[gobid]);
	          enemyspdavg = enemyspdavg + sqrt(distsqrd); // add distance to unit on this frame assuming we didn't move
			  // assume we didn't move because this is ENEMY speed average, not ours + enemy speed average
		      distsqrd = square(lastx[lastbestid[gobid]] - lastx[gobid]) + square(lasty[lastbestid[gobid]] - lasty[gobid]);
		      enemyspdavg = enemyspdavg - sqrt(distsqrd); // subtract distance to unit on last frame
			  // check if the unit can shoot at us
			  ScriptObj * weapon = gob2->component("weapon");
			  if (weapon && enemyinrange[lastbestid[gobid]] != 2) {
				if (square(*weapon->get_int_ptr("max_ground_range") + *gob->sod.radius + *gob2->sod.radius) >=
					square(real8(*gob->sod.x - *gob2->sod.x)) + square(real8(*gob->sod.y - *gob2->sod.y))) {
				  enemyinrange[lastbestid[gobid]] = *gob->sod.radius / unittypediv; // this enemy unit can shoot at this unit type
				}
			  }
		    }
		  }
		  else {
		    enemyunitcount = enemyunitcount + 1; // count # of opponent units
			enemyunittypecount[*gob->sod.radius / unittypediv] = enemyunittypecount[*gob->sod.radius / unittypediv] + 1;
		  }
	    }
	  }
    }
    enemyspdavg = enemyspdavg / unitcount; // turn sum into average
	idealdistapproach = idealdistapproach + enemyspdavg - 1;
	if (idealdistapproach < 1.2 * maxrange - minrange) { // if getting close to enemy
	  idealdistapproach = 0; // tell code we're no longer approaching
	}
  }
  // *************************************
  // MAIN FOR LOOP TO COMPUTE UNIT ACTIONS
  // *************************************
  FORALL (objs, it) {
    GameObj * gob = (*it)->get_GameObj();
    if (gob && gob->sod.in_game && *gob->sod.x >= 0 && !gob->is_dead()) { // check if unit exists and is alive
      ScriptObj * weapon = gob->component("weapon");
      if (!weapon) continue; // so code doesn't crash in other games
      if (n > 1) { // there are other players
        GameObj * target = 0; // typically is unit in range with least HP
        GameObj * bestunit = 0; // to follow (typically is closest unit)
	    GameObj * besttank = 0; // closest enemy siege tank
	    const uint4 gobid = game.get_cplayer_info().get_id(gob);
		sint4 switchmode = -1; // siege mode status
		if (gob->component("weapon2")) {
		  switchmode = *gob->get_int_ptr("mode");
		}
		if (1 == switchmode || 3 == switchmode) continue; // we can't do anything while switching in or out of siege mode
		sint4 rangemin = 0;
		if (2 == switchmode) {
	      weapon = gob->component("weapon2");
		  rangemin = *gob->component("weapon2")->get_int_ptr("min_ground_range");
		}
        const sintptr * prange = weapon->get_int_ptr("max_ground_range");
		const sintptr *lastfired = weapon->get_int_ptr("last_fired");
		const sintptr *cooldown = weapon->get_int_ptr("cooldown");
        if (!prange) continue;
        const sint4 range2 = square(*prange + *gob->sod.radius);
		//const sint4 nearrange2 = square(*prange * 1.2 + *gob->sod.radius); // distance where we are almost in range
        sint4 besthp = -1;
        real8 bestrange2 = -1;
		real8 besttankrange2 = square(maxrange * 1.2 + *gob->sod.radius);
		sint4 followtanklock = 0;
		sint4 targetdamage = -1; // of target's weapon
		// *************************************
		// CHECK FOR TARGETS AND UNITS TO FOLLOW (could be done much faster)
		// *************************************
        FORS (i, n) { // loop through players
          if (i == cid) continue; // exclude client id
          const Game::ObjCont &opp_objs = game.get_objs(i);
          FORALL (opp_objs, it) {
            GameObj * gob2 = (*it)->get_GameObj();
            if (gob2 && gob2->sod.in_game && *gob2->sod.x >= 0 && !gob2->is_dead()) {
		      const sintptr *hp = gob2->get_int_ptr("hp");
			  sint4 dmg = 0;
			  if (gob2->component("weapon")) {
			    dmg = *gob2->component("weapon")->get_int_ptr("min_damage");
			  }
              real8 d2 =
                square(real8(*gob->sod.x - *gob2->sod.x)) +
                square(real8(*gob->sod.y - *gob2->sod.y));
			  sint4 isbestunit = 0; // to help break up big messy if statement
			  if (-1 == bestrange2) {
			    isbestunit = 1; // unit is the first we found in range
			  }
			  // prefer opponent units with weapons with less than 8 of my units following it
			  // in corner mode prefer opponent tanks in siege mode
			  else if (gob2->component("weapon") &&
				  !(bestunitcount[game.get_cplayer_info().get_id(gob2)] >= 8 &&
				  bestunitcount[game.get_cplayer_info().get_id(bestunit)] < 8) &&
				  !(1 == followtanklock && !(gob2->component("weapon2") && *gob2->get_int_ptr("mode") != 0))) {
				if (/*bestrange2 >= nearrange2 &&*/ d2 < bestrange2) {
				  isbestunit = 1; // unit is closer
				}
				/*else if (bestrange2 < nearrange2 && d2 < nearrange2
					&& *hp < *bestunit->get_int_ptr("hp")) {
				  isbestunit = 1; // unit close by has less hp so get rid of it
				}*/
				else if (!bestunit->component("weapon")) {
				  isbestunit = 1; // previous unit was a game 2 building
				}
				if (cornerst == strategy && 0 == followtanklock &&
					gob2->component("weapon2") && *gob2->get_int_ptr("mode") != 0) {
				  isbestunit = 1; // prioritize tanks in siege mode in corner strategy
				  followtanklock = 1; // continue to prioritize this unit
				}
			  }
			  if (1 == isbestunit) {
			    // found better unit to follow
			    bestunit = gob2;
				bestrange2 = d2;
              }
			  // done looking for units to follow
			  // start looking for closest siege tank for marines to stay away from
			  if (linest == strategy && d2 < besttankrange2 &&
				  enemyunittypecount[1] > unittypecount[1] / 4 && enemyunittypecount[2] > unittypecount[2] / 2 &&
				  !gob->component("weapon2") && gob2->component("weapon2") && *gob2->get_int_ptr("mode") != 0) {
			    // found closer siege tank for marine to stay away from
			    besttank = gob2;
				besttankrange2 = d2;
			  }
			  // done looking for closest siege tank
			  // start looking for target
			  if (d2 <= range2 && d2 >= square(rangemin) && *cooldown <= game_tick - *lastfired &&
				  besttargethp[game.get_cplayer_info().get_id(gob2)] <= *hp && dmg >= targetdamage) {
                // target within range
				if (*hp < besthp || -1 == besthp || (*hp < *weapon->get_int_ptr("min_damage") && *hp > besthp)) {
				  // target has less hp then next best target
                  target = gob2;
				  besthp = *hp;
				}
				if (target->component("weapon")) { // in case it is a game 2 building
				  if (dmg >= *target->component("weapon")->get_int_ptr("max_damage")) {
				    // aim for tanks over marines
				    target = gob2;
				    besthp = *hp;
					targetdamage = dmg;
				  }
				}
			  }
			  // keep track of current opponent unit positions to reference next frame
		      lastx[game.get_cplayer_info().get_id(gob2)] = *gob2->sod.x;
		      lasty[game.get_cplayer_info().get_id(gob2)] = *gob2->sod.y;
			  if (!gob2->component("weapon")) {
			    idealdistapproach = 0; // this means we're in game 2 where the fancy approach doesn't help
			  }
            }
          }
        } // end looking for targets and units to follow
		if (!bestunit) continue;
		real8 bestrange = sqrt(bestrange2);
		bestrangesum = bestrangesum + bestrange; // to find best range average later on
		/*if (bestrange < bestrangeminnew) {
		  bestrangeminnew = bestrange;
		}*/
		// keep track of # of units following opponent units
		if (gob->component("weapon2") || !bestunit->component("weapon2")) { // any # of marines may follow tanks
		  bestunitcount[game.get_cplayer_info().get_id(bestunit)] = bestunitcount[game.get_cplayer_info().get_id(bestunit)] + 1;
		}
		if (0 == switchmode && ((cornerst == strategy && game_tick >= switchtime && (game_tick > 300 ||
			square(real8(*gob->sod.x)) + square(real8(*gob->sod.y)) < square(*gob->sod.radius * 12) ||
			square(real8(*gob->sod.x - points_x)) + square(real8(*gob->sod.y)) < square(*gob->sod.radius * 12))) ||
			(strategy != cornerst && strategy != linenosiegest && bestrange <= maxrange * 1 && bestrange > *prange && 0 == idealdistapproach))) {
		  // switch into siege mode when in range or in corner
          Vector<sint4> args;
          gob->set_action("switch", args);
		  //cout << "switch" << endl;
		  continue;
		}
		else if (strategy != cornerst && 2 == switchmode && (bestrange > maxrange || bestrange < rangemin)) {
		  // switch out of siege mode if out of range
          Vector<sint4> args;
          gob->set_action("switch", args);
		  //cout << "switch back" << endl;
		  continue;
		}
        if (target) {
          //cout << "attack" << endl;
		  // *************
		  // ATTACK TARGET
		  // *************
          //cout << "launch " << sqrt(min_d) << " " << sqrt(attack_d) << endl;
          Vector<sint4> args;
		  besttargethp[game.get_cplayer_info().get_id(target)] = besttargethp[game.get_cplayer_info().get_id(target)] +
			  *weapon->get_int_ptr("min_damage"); // keep track of minimum damage to this unit
		  if (target->get_int_ptr("armor")) {
		    besttargethp[game.get_cplayer_info().get_id(target)] = besttargethp[game.get_cplayer_info().get_id(target)] -
				*target->get_int_ptr("armor"); // armored targets take less damage
		  }
		  if (2 == switchmode) { // tanks that have switched do special attack (with splash)
			args.push_back(*target->sod.x);
            args.push_back(*target->sod.y);
            gob->component("weapon2")->set_action("attack", args);
			//cout << "special atk" << endl;
		  }
		  else { // otherwise do regular attack
            args.push_back(game.get_cplayer_info().get_id(target));
            gob->component("weapon")->set_action("attack", args);
		  }
          continue;
        }
		/*else if (1 == hasswitched[gobid] && bestrange2 > range2 * 1.3) {
		  hasswitched[gobid] = 2; // don't switch again after this
		  allswitch = allswitch + 1; // tanks in siege mode that are out of range should switch back
		}
		if (allswitch >= 5 && hasswitched[gobid] != 3) {
		  if (hasswitched != 0) {
		    Vector<sint4> args;
		    gob->set_action("switch", args); // all switch back at the same time
		    cout << "all switching" << endl;
		  }
		  hasswitched[gobid] = 3;
		  continue;
		}*/
		// ********************
		// MOVE IF NOT SHOOTING
		// ********************
		// get ideal distance to be from opponent unit that we're following
		real8 idealdist = 1.2; // be out of range while reloading
		if (idealdistapproach > 0) {
		  idealdist = idealdistapproach / *prange + 1; // use global variable when still approaching
		}
		else if (!gob->component("weapon2") && bestunit->component("weapon2") && *bestunit->get_int_ptr("mode") != 0) {
		  idealdist = 0; // marines try to get within minimum siege tank range
		}
	    else if (*cooldown <= game_tick - *lastfired) {
		  idealdist = 1; // be in range when done reloading
		}
		/*else if (bestunit->component("weapon")) { // in case it is a game 2 building
		  if (gob->component("weapon")->get_int_ptr("min_damage") > bestunit->component("weapon")->get_int_ptr("max_damage")) {
		    idealdist = 1; // tanks may get closer to marines
		  }
		}*/
		if (bestrange * idealdist != *prange) {
          // object not dead and speed == 0 and not in range => move action
          ScalarPoint t; // final position to go to
          sint4 bestx; // best x to be at
          sint4 besty; // best y to be at
		  sint4 chgxy = 0; // set this to 1 to tell other parts of code that something changed
		  // find best position to be at
		  // ********************************
		  // FIND BEST POSITION FOR LINE MODE
		  // ********************************
		  if (strategy != cornerst || (cornerst == strategy && 0 == idealdist)) { // for line mode or charging marines
		    // be certain distance from unit we're following
		    bestx = lineatdist(prange, idealdist, bestrange, *gob->sod.x, *bestunit->sod.x);
		    besty = lineatdist(prange, idealdist, bestrange, *gob->sod.y, *bestunit->sod.y);
			sint4 idealtankrange = (maxrange + *gob->sod.radius) + (*prange + 0.2);
			if (besttank && enemyinrange[game.get_cplayer_info().get_id(bestunit)] != 2
				&& besttankrange2 <= square(idealtankrange)) {
			  // marines stay out of range of opponent siege tanks
			  besttankrange2 = square(real8(bestx - *besttank->sod.x)) + square(real8(besty - *besttank->sod.y));
			  bestx = lineatdist(prange, idealtankrange / *prange,
				  sqrt(besttankrange2), bestx, *besttank->sod.x);
		      besty = lineatdist(prange, idealtankrange / *prange,
				  sqrt(besttankrange2), besty, *besttank->sod.y);
			}
		  }
		  // **********************************
		  // FIND BEST POSITION FOR CORNER MODE
		  // **********************************
		  else if (cornerst == strategy) { // for corner mode
			bestx = 140 * (1 - game_tick / real8(switchtime - 30)) + 80;
			if (bestx < 80) { // i.e. if it has passed 200 frames
			  // marines move to 2.3 times their range minus the ideal distance calculated earlier from the corner
			  bestx = 0;
			  besty = 0;
			  idealdist = 2.3 - idealdist;
			}
			else { // if in 1st 200 frames
		      // marines form a vertical line based on their order calculated on the 1st frame
			  // that moves from 140 units to 80 units away from the edge over a period of 100 frames
		      besty = unit2linepos[gobid] * *gob->sod.radius * 2.5 + *gob->sod.radius + 1;
			  idealdist = 0;
			}
			const sint4 bestxline = bestx + idealdist * minrange; // x value that tanks should not go past
		    if (*gob->sod.x > points_x / 2) {
		      bestx = points_x - bestx; // make position for correct corner
		    }
		    bestrange2 = square(real8(*gob->sod.x - bestx)) + square(real8(*gob->sod.y - besty));
			bestrange = sqrt(bestrange2);
			if (gob->component("weapon2")) {
			  // tanks form a square formation of units starting with units spaced 8 times their radius apart,
			  // slowly decreasing for 230 frames
			  idealdist = 8 * (1 - game_tick / real8(switchtime));
			  if (idealdist < 1) {
			    idealdist = 1;
			  }
			  bestx = (unit2linepos[gobid] % int(sqrt(real8(unittypecount[*gob->sod.radius / unittypediv])))) * *gob->sod.radius * idealdist + *gob->sod.radius + 1;
			  real8 bestyinprogress = unit2linepos[gobid] / sqrt(real8(unittypecount[*gob->sod.radius / unittypediv]));
			  besty = bestyinprogress;
			  if (besty > bestyinprogress) {
			    besty = besty - 1; // my way of rounding down
			  }
			  besty = besty * *gob->sod.radius * idealdist + *gob->sod.radius + 1;
			  if (*gob->sod.x > points_x / 2) {
			    bestx = points_x - bestx; // go to right corner if we are on right side
				if (*gob->sod.radius * 2 + points_x - *gob->sod.x >= bestxline && besty < *gob->sod.y) {
				  besty = *gob->sod.y + 1; // emphasize moving past marine line
				}
			  }
			  else if (*gob->sod.radius * 2 + *gob->sod.x >= bestxline && besty < *gob->sod.y) {
			    besty = *gob->sod.y + 1; // emphasize moving past marine line
			  }
			}
			else { // marines form line around tanks
			  bestx = lineatdist(prange, idealdist, bestrange, *gob->sod.x, bestx);
		      besty = lineatdist(prange, idealdist, bestrange, *gob->sod.y, besty);
			}
		  } // end finding corner mode best position
		  // ***********************************************************
		  // STAY ACCEPTABLE DISTANCE FROM NEIGHBOR CLOSER TO THE CENTER (on initial approach)
		  // ***********************************************************
		  sint4 adjustunit = unittypecount[*gob->sod.radius / unittypediv] / 2;
		  if (adjustunit == unit2linepos[gobid] && game_tick < 100 && strategy != cornerst) {
		    besty = points_y / 2; // unit in center should try to go to center at first
		  }
		  else if (unit2linepos[gobid] != adjustunit && game_tick > 30 && strategy != cornerst) {
		    // stay at an acceptable distance from the unit in the center
		    sint4 adjunitrel = 1;
			if (unit2linepos[gobid] > adjustunit) {
			  adjunitrel = -1;
			}
		    ScriptObj * adjunit = 0;
			sint4 adjunitpos = unit2linepos[gobid];
			while (!adjunit || !adjunit->get_GameObj()) { // in case this unit is dead
			  adjunitpos = adjunitpos + adjunitrel;
			  adjunit = game.get_cplayer_info().get_obj(linepos2unit[adjunitpos][*gob->sod.radius / unittypediv]);
		      if (adjunit && adjunit->get_GameObj()) {
		        GameObj * gob2 = adjunit->get_GameObj();
			    real8 dsin = real8(besty - *gob2->sod.y);
			    real8 dcos = real8(bestx - *gob2->sod.x);
			    real8 dist = sqrt(square(dcos) + square(dsin));
			    dsin = dsin / dist;
			    dcos = dcos / dist;
			    // go perpendicular to current movement
			    /*if ((dist < (*gob->sod.radius + *gob2->sod.radius) * 1.5 && 1 == adjunitrel) ||
				    (dist > (*gob->sod.radius + *gob2->sod.radius) * 2 && -1 == adjunitrel && idealdistapproach > 0)) {
				  bestx = bestx + dsin * 2;
				  besty = besty - dcos * 2;
			    }
			    else if ((dist < (*gob->sod.radius + *gob2->sod.radius) * 1.5 && -1 == adjunitrel) ||
				    (dist > (*gob->sod.radius + *gob2->sod.radius) * 2 && 1 == adjunitrel && idealdistapproach > 0)) {
				  bestx = bestx - dsin * 2;
				  besty = besty + dcos * 2;
			    }*/
			    if (dist < (*gob->sod.radius + *gob2->sod.radius) * 1.5) {
			      // go away from neighbor if too close
				  real8 rsin = real8(besty - *bestunit->sod.y);
				  real8 rcos = real8(bestx - *bestunit->sod.x);
				  dist = sqrt(square(rcos) + square(rsin)) / (*gob->sod.radius * 2);
				  if (dist >= 1) { // only respond if we are far enough away from ideal position
				    rsin = rsin / dist;
				    rcos = rcos / dist;
				    // try going perpendicular to current movement
				    sint4 bestxtest = bestx + (rsin * adjunitrel);
				    sint4 bestytest = besty - (rcos * adjunitrel);
				    if (square(bestxtest - *gob2->sod.x) + square(bestytest - *gob2->sod.y) <
						square(bestx - (rsin * adjunitrel) - *gob2->sod.x) + square(besty + (rcos * adjunitrel) - *gob2->sod.y)) {
					  // go directly away from unit if that didn't work
				      bestx = bestx + dcos * 2;
				      besty = besty + dsin * 2;
					}
				  }
				  else {
				    // code should have crashed due to really strange event if we got here
				    bestx = bestx + dcos * 2;
				    besty = besty + dsin * 2;
				  }
			    }
			    else if (dist > (*gob->sod.radius + *gob2->sod.radius) * 3 && idealdistapproach > 0 && 0 == approachcount) {
			      // go towards neighbor if too far away
				  bestx = bestx - dcos * 4;
				  besty = besty - dsin * 4;
			    }
			    /* math.ucsd.edu/~wgarner/math4c/derivations/distance/distptline.htm
1 = pt, 2/3 = line
y = (y2 - y3) / (x2 - x3) * (x - x2) + y2
m = (y2 - y3) / (x2 - x3)
b = y2 - m * x2
abs(y1 - (y2 - y3) / (x2 - x3) * x1 - y2 + (y2 - y3) / (x2 - x3) * x2) / sqr(((y2 - y3) / (x2 - x3)) ^ 2 + 1
abs(y1 - y2 - (y2 - y3) / (x2 - x3) * (x2 - x1)) / sqr(((y2 - y3) / (x2 - x3)) ^ 2 + 1) */
			  }
			}
		  }
		  // **************************
		  // ALTERNATE WAY TO FORM LINE (after initial approach)
		  // **************************
		  // helps avoid clumps of units
		  // note that we no longer use order calculated on 1st frame since it's likely outdated
		  // if approaching again and close enough to best position
		  if (idealdistapproach > 0 && 1 == approachcount && square(real8(*gob->sod.x - bestx)) + square(real8(*gob->sod.y - besty)) < square(*gob->sod.radius * 10)) {
			real8 dsin = real8(*gob->sod.y - besty);
			real8 dcos = real8(*gob->sod.x - bestx);
			real8 dist = sqrt(square(dcos) + square(dsin)) / (*gob->sod.radius * 2);
			if (dist >= 1) { // if we are far enough away from ideal position
              // check if someone is at best position
              const Game::ObjCont &col_objs = game.get_objs(cid);
              FORALL (col_objs, it) { // loop through objects owned by this player to check if someone is at best position
                GameObj * gob2 = (*it)->get_GameObj();
			    if (gob2 && gob2->sod.in_game && *gob2->sod.x >= 0 && !gob2->is_dead() && (*gob->sod.x != *gob2->sod.x || *gob->sod.y != *gob2->sod.y)) {
				  real8 dist2 = square(real8(bestx - *gob2->sod.x)) + square(real8(besty - *gob2->sod.y));
				  if (dist2 <= square((*gob->sod.radius + *gob2->sod.radius))) {
				    // someone's at best position, need to move somewhere else
					dsin = dsin / dist;
					dcos = dcos / dist;
					// go perpendicular to current movement (todo?: move to side with less units)
					bestx = *gob->sod.x + dsin;
					besty = *gob->sod.y - dcos;
					/*if (square(t.x - *gob2->sod.x) + square(t.y - *gob2->sod.y) < square(*gob->sod.x - dsin - *gob2->sod.x) + square(*gob->sod.y + dcos - *gob2->sod.y)) {
					  bestx = *gob->sod.x - dsin;
					  besty = *gob->sod.y + dcos;
				    }*/
				    break; // i.e. exit loop
				  }
				}
			  }
			}
		  }
		  // *****************************
		  // DISCOURAGE GOING OFF MAP EDGE
		  // *****************************
		  if (strategy != cornerst) {
		    chgxy = 0;
		    if (bestx > points_x - *gob->sod.radius) {
		      bestx = points_x - *gob->sod.radius;
			  chgxy = 1;
			  //cout << ">width" << endl;
		    }
		    else if (bestx < *gob->sod.radius) {
		      bestx = *gob->sod.radius;
			  chgxy = 1;
			  //cout << "<width" << endl;
		    }
		    if (1 == chgxy) {
			  if (besty > *gob->sod.y) {
			    besty = points_y - *gob->sod.radius;
			  }
			  else {
			    besty = *gob->sod.radius;
			  }
		    }
		    chgxy = 0;
		    if (besty > points_y - *gob->sod.radius) {
		      besty = points_y - *gob->sod.radius;
			  chgxy = 1;
			  //cout << ">height" << endl;
		    }
		    else if (besty < *gob->sod.radius) {
		      besty = *gob->sod.radius;
			  chgxy = 1;
			  //cout << "<height" << endl;
		    }
		    if (1 == chgxy) {
			  if (bestx > *gob->sod.x) {
			    bestx = points_x - *gob->sod.radius;
			  }
			  else {
			    bestx = *gob->sod.radius;
			  }
		    }
		  }
		  // ********************************
          // COLLISION DETECTION AND RESPONSE
		  // ********************************
		  if (0 == *gob->sod.speed && 0 == firstframe && (*gob->sod.x != bestx || *gob->sod.y != besty)) { // if not moving
		    t.x = 0;
			t.y = 0;
			FORS (i, n) { // loop through players
              const Game::ObjCont &col_objs = game.get_objs(i);
              FORALL (col_objs, it) { // loop through objects owned by this player
                GameObj * gob2 = (*it)->get_GameObj();
				if (gob2 && gob2->sod.in_game && *gob2->sod.x >= 0 && !gob2->is_dead() && /* if alive */
					(*gob->sod.x != *gob2->sod.x || *gob->sod.y != *gob2->sod.y)) { // defensive programming
				  real8 dsin = real8(*gob->sod.y - *gob2->sod.y);
				  real8 dcos = real8(*gob->sod.x - *gob2->sod.x);
				  real8 dist = square(dcos) + square(dsin);
				  if (dist <= square((*gob->sod.radius + *gob2->sod.radius) * 1.1)) {
					// collision, need to respond
				    if (1 == strategy || square(*gob->sod.x - bestx) + square(*gob->sod.y - besty) > square(*gob->sod.radius * 10)) {
					  // calculate distance from best position if far away from it or in corner mode
					  dsin = real8(*gob->sod.y - besty);
				      dcos = real8(*gob->sod.x - bestx);
					}
					else {
					  // otherwise calculate distance from unit we're following
				      dsin = real8(*gob->sod.y - *bestunit->sod.y);
				      dcos = real8(*gob->sod.x - *bestunit->sod.x);
					}
				    dist = sqrt(square(dcos) + square(dsin)) / (*gob->sod.radius * 2);
					if (dist >= 1) { // only respond if we are far enough away from ideal position
					  dsin = dsin / dist;
					  dcos = dcos / dist;
					  // go perpendicular to current movement going away from other unit
					  t.x = *gob->sod.x + dsin;
					  t.y = *gob->sod.y - dcos;
					  if (square(t.x - *gob2->sod.x) + square(t.y - *gob2->sod.y) < square(*gob->sod.x - dsin - *gob2->sod.x) + square(*gob->sod.y + dcos - *gob2->sod.y)) {
					    t.x = *gob->sod.x - dsin;
					    t.y = *gob->sod.y + dcos;
					  }
					}
					break; // i.e. exit loop
				  }
				}
			  }
			  if (t.x != 0 || t.y != 0) break;
			}
			if (0 == t.x && 0 == t.y) {
			  // move randomly if we didn't find a collision (partly for game 2 terrain)
			  t.x = rand.rand_uint4() % points_x;
			  t.y = rand.rand_uint4() % points_y;
			}
		  }
		  else {
		    // no collision, go to ideal position
		    t.x = bestx;
		    t.y = besty;
		  }
		  // set move command
          Vector<sint4> params;
          params.push_back(t.x);
          params.push_back(t.y);
		  //if (bestrange > bestrangeavg || bestrangeavg < 150 || idealdist > bestrange / *prange) {
          params.push_back(*gob->sod.max_speed);
		  /*}
		  else if (bestrange > bestrangemin) {
		    params.push_back(*gob->sod.max_speed * 2 / 3);
		  }
		  else { // if ahead of everyone else then wait for them to catch up
		    params.push_back(*gob->sod.max_speed / 3);
		  }*/
          gob->set_action("move", params);
        }
		// remember some properties to check how enemy moves
	    lastbestid[gobid] = game.get_cplayer_info().get_id(bestunit);
	    lastx[gobid] = *gob->sod.x;
	    lasty[gobid] = *gob->sod.y;
      }
    }
  }
  // - global player/game variables
  // (to end the frame)
  if (0 == game_tick % 20) {
    cout << unitcount << "vs" << enemyunitcount << endl;
  }
  // check if we win or lose by computing # of our units and enemy units
  if ((0 == unitcount || 0 == enemyunitcount) && usefile > 0 &&
	  -1 == gameended && 0 == firstframe) {
    if (unitcount >= enemyunitcount) {
      gameended = 1; // we win
	}
	else {
	  gameended = 0; // we lose
	}
  }
  if (unitcount > 0) {
    bestrangeavg = bestrangesum / unitcount;
  }
  //bestrangemin = bestrangeminnew;
  firstframe = 0;
}

// load strategy success info from data file for learning algorithm
// returns 1 if success, 0 if fail
sint4 loaddatafile()
{
  ifstream file;
  file.open(datapath.c_str());
  if (file.is_open()) {
    FORS (i, nstrategy) {
	  if (file.good()) {
	    file >> totalwins[i];
	  }
	  if (file.good()) {
	    file >> totalgames[i];
	  }
	  if (totalgames[i] <= 0) {
	    cout << "REMARK: strategy " << i << " game total claims to be " << totalgames[i] << endl;
	    totalgames[i] = 1;
	  }
	  if (totalwins[i] > totalgames[i]) {
	    cout << "REMARK: strategy " << i << " win rate claims to be "
		    << totalwins[i] << "/" << totalgames[i] << endl;
	    totalwins[i] = totalgames[i];
	  }
    }
  }
  else {
	return 0;
  }
  file.close();
  return 1;
}

// save strategy success info to data file for learning algorithm
// automatically locks file, checks for updated stats, and prints information
// thanks to cplusplus.com/doc/tutorial/files.html
void savedatafile()
{
  if (2 == gameended) return; // in case we already saved
  sint4 safetoopen = lockdatafile(); // check lock file in case another process is using it
  if (safetoopen >= 0) {
    if (1 == loaddatafile() || (1 == totalgames[0] && 1 == totalgames[nstrategy - 1])) {
      // no other process is using the file so safe to save to
      // todo: consider client ids (reduce starting values so can override later)
      ofstream file;
      file.open(datapath.c_str());
      if (file.is_open()) {
        totalgames[firststrategy] = totalgames[firststrategy] + 1;
	    totalwins[firststrategy] = totalwins[firststrategy] + gameended;
	    FORS (i, nstrategy) {
	      file << totalwins[i] << endl;
	      file << totalgames[i] << endl;
	    }
	    cout << "REMARK: save data file succeeded" << endl;
      }
      else {
        perror("REMARK: error saving data file");
      }
      file.close();
    }
    else {
      perror("REMARK: error checking for latest stats");
    }
  }
  else {
    perror("REMARK: error opening lock file");
  }
#ifdef LINUX_MODE // todo: line below gets permission denied error on Windows
  if (std::remove(lockpath.c_str()) != 0) perror("REMARK: cannot delete lock file");
#endif
  // print some information about the strategies
  if (1 == gameended) {
    cout << "strategy " << firststrategy << " wins" << endl;
  }
  else {
    cout << "strategy " << firststrategy << " loses" << endl;
  }
  if (strategy != firststrategy) {
    cout << "strategy changed to " << strategy << " mid-game" << endl;
  }
  FORS (i, nstrategy) {
    cout << "strategy " << i << " win rate: " << totalwins[i] << "/" << totalgames[i]
	    << " (" << (totalwins[i] * 100 / totalgames[i]) << "%)" << endl;
  }
  gameended = 2;
}

// wait until other processes are not using data file then reserve it
// returns >= 0 if success
// thanks to gethelp.devx.com/techtips/cpp_pro/10min/2002/April/10min0402-2.asp
sint4 lockdatafile()
{
#ifndef LINUX_MODE
  return 1;
#endif
  sint4 failcount = 0;
  sint4 fd = -1; // fd stands for file descriptor
  while (fd < 0 && failcount < 500000) {
    fd = open(lockpath.c_str(), O_WRONLY | O_CREAT | O_EXCL);
    if (fd < 0 && EEXIST != errno) break; // error but no other process is using this file
	failcount = failcount + 1;
  }
  return fd;
}

// returns next line in open file
// thanks to MD3_Model::load_cfg
/*string getfileline(ifstream file)
{
  char cline[256];
  file.getline(cline, 256);
  string line = cline;
  FORU (i, line.size()) {
    if (line[i] == '\r' || line[i] == '\n') {
      line = line.substr(0, i);
      break;
    }
  }
  return line;
}*/

// returns x/y that is prange * idealdist from followpos if traveling from currentpos
inline sint4 lineatdist(const sintptr *prange, real8 idealdist, real8 currentdist, sint4 currentpos, sint4 followpos)
{
  return *prange * idealdist / currentdist * (currentpos - followpos) + followpos;
}
