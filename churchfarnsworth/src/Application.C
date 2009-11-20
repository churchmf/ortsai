#include "Application.H"

#include "Global.H"
#include "GfxGlobal.H"
#include "Options.H"
#include "GfxModule.H"
#include "GameStateModule.H"
#include "ShutDown.H"
#include "SDLinit.H"
#include "Game.H"
#include "GUI.H"
#include "DrawOnTerrain2D.H"

// Constants
static const char *	VERSION	= "Orts Sample Application";
static const sint4	GFX_X		= 900;
static const sint4	GFX_Y		= 740;

Application::Application() :
	gameState(0), debugDisplay(0), graphicsClient(0),
	just_drew(false), quit(false), error(false), refresh(false)
{
	// If running on Unix/Linux, ignore signals
	signals_shut_down(true);
}

Application::~Application()
{
	if(graphicsClient) delete graphicsClient;
	if(debugDisplay) delete debugDisplay;
	if(gameState) delete gameState;
}

int Application::Run(int argc, char * argv[])
{
	// Populate options native to this application
	Options opt("OrtsApplication");
	opt.put("-disp",	"Use 2D debug display");
	opt.put("-usegfx",	"Use ORTSG graphical client");

	// Populate options used by our modules
	Game::Options::add();				// Used for core game data (units, map)
	GameStateModule::Options::add();	// Used for network synchronisation
	GUI::add_options();					// Used for 2D debug display
	GfxModule::Options::add();			// Used for ORTSG

	// Process command line options
	if(Options::process(argc, argv, std::cerr, VERSION)) return -20;

	// If no random seed specified, seed with current system time
	sint4 seed;
	Options::get("-seed", seed);
	if (!seed) seed = time(0);
	Options::set("-seed", seed);
	std::cout << "seed=" << seed << std::endl;

	// If the user has specified a graphical client, instantiate it
	bool usegfx;
	Options::get("-usegfx", usegfx);
	if(usegfx)
	{
		Options::get("-refresh", refresh);
		graphicsClient = new GfxModule;
	}

	// Connect to server
	SDLinit::network_init();
	gameState = new GameStateModule(GameStateModule::Options());
	gameState->add_handler(this);
	if(!gameState->connect())
	{
		ERR("connection problems");
	}

	// If the user has specified a 2D debug display, open it
	bool disp;
	Options::get("-disp", disp);
	if(disp)
	{
		SDLinit::video_init();
		debugDisplay = new GUI;
		debugDisplay->init(GFX_X, GFX_Y, gameState->get_game());
		debugDisplay->display();
	}

	// Any user initialization
	Initialize();

	// If the user has specified the graphical client, start it
	if(usegfx)
	{
		// Initialise the graphical client
		glutInit(&argc, argv);
		graphicsClient->init(
			gameState->get_game(),
			gameState->get_changes(),
			gameState->get_action_changes(),
			GfxModule::Options());

		// Pass control to GLUT
		ApplicationLooper looper(*this);
		graphicsClient->start_loop(&looper);
	}
	else
	{
		// Manually drive our main loop
		while(!quit)
		{
			OnUpdate();
		}

		//do cleanup
	}
	return 0;
}

void Application::DrawDebugLine(const vec2 & start, const vec2 & end, const Color & color)
{
	if(!debugDisplay) return;

	DebugLine line;
	line.p0 = start;
	line.p1 = end;
	line.c	= color;
	debugLines.push_back(line);
}

void Application::DrawDebugCircle(const vec2 & center, sint4 radius, const Color & color)
{
	if(!debugDisplay) return;

	DebugCircle circle;
	circle.p = center;
	circle.r = radius;
	circle.c = color;
	debugCircles.push_back(circle);
}

// Main loop, called by GLUT
void Application::OnUpdate()
{
	if(quit)
	{
		if(graphicsClient) exit(0);
		return;
	}

	// If we are running the graphical client
	if(graphicsClient)
	{
		// If the user has asked to quit
		if(graphicsClient->get_quit())
		{
			// Quit the program
			quit = true;
		}

		// If we are rendering every update
		if(refresh)
		{
			// If we just drew (in response to receiving a view)
			if(just_drew)
			{
				// Do not render this update
				just_drew = false;
			}
			else
			{
				// Render a frame
				graphicsClient->draw();
			}
		}
	}

	// Wait for a view to arrive
	if(!gameState->recv_view())
	{
		SDL_Delay(1);
	}
}

bool Application::handle_event(const Event & e)
{
	if(quit) return false;

	// Only handle events from the GameStateModule
	if(e.get_who() == GameStateModule::FROM)
	{
		// Handle STOP_MSG and FINISHED_MSG (the game is over)
		if(e.get_what() == GameStateModule::STOP_MSG ||
		   e.get_what() == GameStateModule::FINISHED_MSG)
		{
			quit = true;
			return true;
		}

		// Handle READ_ERROR_MSG (error occurred receiving data)
		if(e.get_what() == GameStateModule::READ_ERROR_MSG)
		{
			quit = error = true;
			return true;
		}

		// Handle VIEW_MSG (a new view has been received)
		if(e.get_what() == GameStateModule::VIEW_MSG)
		{
			// Send actions
			OnReceivedView(*gameState);
			gameState->send_actions();

			// Compute graphics and frame statistics
			const Game & game(gameState->get_game());
			const sint4 vf(game.get_view_frame());
			const sint4 af(game.get_action_frame());
			const sint4 sa(game.get_skipped_actions());
			const sint4 ma(game.get_merged_actions());

			if((vf % 20) == 0)
			{
				std::cout << "[frame " << vf << "]" << std::endl;
			}

			if(vf != af || sa || ma > 1)
			{
				std::cout << "frame " << vf;
				if(af < 0)
				{
					std::cout << " [no action]";
				}
				else if(vf != af)
				{
					std::cout << " [behind by " << (vf - af) << " frame(s)]";
				}
				if(sa)
				{
					std::cout << " [skipped " << sa << "]";
				}
				if(ma > 1)
				{
					std::cout << " [merged " << ma << "]";
				}
				std::cout << std::endl;
			}

			// Render graphics if necessary
			if(graphicsClient)
			{
				graphicsClient->process_changes();

				// Do not draw if we are far behind
				if(abs(vf-af) < 10)
				{
					graphicsClient->draw();
					just_drew = true;
				}
			}

			// Handle debug display
			if(debugDisplay)
			{
				debugDisplay->event();
				if(debugDisplay->quit)
				{
					quit = true;
					return true;
				}

				// As a hack for flickering, gui->display() is called in dot.start()
				// Therefore, call everything that draws to the debug window between start() and end()
				DrawOnTerrain2D draw2d(debugDisplay);
				draw2d.start();
				for(size_t i(0); i<debugLines.size(); ++i)
				{
					const DebugLine & line(debugLines[i]);
					draw2d.draw_line(line.p0.x, line.p0.y, line.p1.x, line.p1.y, Vec3<real4>(line.c.r, line.c.g, line.c.b));
				}
				for(size_t i(0); i<debugCircles.size(); ++i)
				{
					const DebugCircle & circle(debugCircles[i]);
					draw2d.draw_circle(circle.p.x, circle.p.y, circle.r, Vec3<real4>(circle.c.r, circle.c.g, circle.c.b));
				}
				draw2d.end();

				debugLines.clear();
				debugCircles.clear();
			}

			return true;
		}
	}

	return false;
}

void ApplicationLooper::loop() const
{
	app.OnUpdate();
}
