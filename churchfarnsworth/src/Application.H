#ifndef APPLICATION_H
#define APPLICAITON_H

#include "EventHandler.H"
#include "LoopFunctor.H"
#include "Helper.H"
#include "Movement.H"

class Application;
class GameStateModule;
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

	GameStateModule *	gameState;
	Movement::Module::ptr	mm;
	Movement::Context *		mc;
	GUI *				debugDisplay;
	GfxModule *			graphicsClient;

	bool				just_drew;  // Have we just drawn a gfx frame?
	bool				quit;		// Have we quit?
	bool				error;      // Has an error occurred?
	bool				refresh;	// Should we render every frame?

	Vector<DebugLine>	debugLines;
	Vector<DebugCircle>	debugCircles;

	void OnUpdate();				// Game Update Loop, called

protected:
	void DrawDebugLine(const vec2 & start, const vec2 & end, const Color & color);
	void DrawDebugCircle(const vec2 & center, sint4 radius, const Color & color);
public:
	Application();
	~Application();

	virtual void OnReceivedView(GameStateModule & gameState, Movement::Context&) = 0;

	int Run(int argc, char * argv[]);

	bool handle_event(const Event & event);	// Required to implement EventHandler
};

#endif
