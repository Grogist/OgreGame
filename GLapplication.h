/***************************



***************************/

#ifndef __Application_H__
#define __Application_H__

#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreWindowEventUtilities.h>
#include <fmod.hpp>
#include <fmod_errors.h>

#include "GLcamera.h"
#include "GLgame.h"
#include "GLunit.h"
#include "GLinput.h"
#include "GLgame.h"
#include "GLGUI.h"
#include "GLlocation.h"
#include "GLSoundManager.h"
#include "GLMinimap.h"

namespace GAME{
class GameState;
}
class Application : public Ogre::WindowEventListener, public Ogre::FrameListener
{
public:
	Application(void);
	~Application(void);

	Ogre::Camera *getCamera(void);
	Ogre::Viewport *getViewport(void);
	GAME::GameInput *getInput(void);
	GAME::GUI *getGUI(void);
	Ogre::Root *getRoot(void);

	bool start(GAME::GameState* state);
	void changeState(GAME::GameState* state);
	void pushState(GAME::GameState* state);
	void popState();

	GAME::GameState *getGameState(void);

	void setShutdown(bool shutdown);

	GAME::SoundManager *soundMgr;
	
protected:
	virtual void windowResized(Ogre::RenderWindow *rw);
	virtual void windowClosed(Ogre::RenderWindow *rw);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

private:

	void init(void);

	Ogre::Root *Root;
	Ogre::String PluginsCfg;
	Ogre::String ResourcesCfg;
	Ogre::RenderWindow *Window;

	GAME::GameInput *input;

	GAME::GUI *theGUI;

	//GAME::Minimap minimap;

	bool Shutdown;
	bool gamePaused;

	int sound;
	int channel;
	//int songLocation;
	int count;
	FMOD_TAG tag;
	FMOD_RESULT result;
	char file[128];
	//std::vector<std::string> songShuffleList;
	FMOD::Sound *playlist;

	std::vector<GAME::GameState *> m_States;
};

extern Application TheApplication;

#endif