#ifndef ORTS_SAMPLE_APPLICATION_H
#define ORTS_SAMPLE_APPLICAITON_H

#include "EventHandler.H"
#include "LoopFunctor.H"
#include "Movement.H"
#include "Helper.H"

class Application;
class GameStateModule;
class GameChanges;
class Game;
class GUI;
class GfxModule;

class ApplicationLooper : public LoopFunctor
{
	Application & app;
public:
	ApplicationLooper(Application & app) : app(app) {}

	void loop() const;
};	

class Application : public EventHandler
{
	friend class ApplicationLooper;

	struct DebugLine { vec2 p0; vec2 p1; Color c; };
	struct DebugCircle { vec2 p; sint4 r; Color c; };

	GameStateModule *		gameState;
	GUI *					debugDisplay;
	GfxModule *				graphicsClient;
	Movement::Module::ptr	moveModule;
	Movement::Context *		moveContext;

	bool					justDrew;	// Have we just drawn a gfx frame?
	bool					quit;		// Have we quit?
	bool					error;      // Has an error occurred?
	bool					refresh;	// Should we render every frame?

	std::vector<Unit::ptr>	units;		// All game units currently in play

	Vector<DebugLine>		debugLines;
	Vector<DebugCircle>		debugCircles;

	void OnUpdate();
	void HandleChanges();
protected:
	void DrawDebugLine(const vec2 & start, const vec2 & end, const Color & color);
	void DrawDebugCircle(const vec2 & center, sint4 radius, const Color & color);
public:
	Application();
	~Application();

	virtual void		OnNewUnit(Unit::ptr unit)	= 0;
	virtual void		OnUnitDeath(Unit::ptr unit)	= 0;
	virtual void		OnReceivedView()			= 0;

	int					Run(int argc, char * argv[]);

	GameStateModule &	GetGameState();
	const GameChanges &	GetGameChanges();
	Game &				GetGame();
	Movement::Context &	GetMoveContext();

	bool handle_event(const Event & event);	// Required to implement EventHandler
};

#endif
