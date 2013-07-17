/***********************************************************************

File: GLGameState.h
Author: Gregory Libera

Description: GameState is an abstract class that dictates the structure
of the different states the game can be in. Only PlayState and MenuState
inherit from GameState. Each GameState must be Singleton as only at most
one of each state may exist at any given time.

***********************************************************************/

#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include <Ogre.h>
#include <OIS.h>

#include "GLapplication.h"

namespace GAME
{
	class GameState
	{
	public:
		// Is called when entering a GameState;
		virtual void Init(void) = 0;
		// Is called when leaving a GameState
		virtual void Cleanup(void) = 0;

		// Pauses and Resumes a GameState
		virtual void Pause(void) = 0;
		virtual void Resume(void) = 0;

		// Captures and responds to keyboard input.
		// Capture is called per frame.
		virtual void capture(Ogre::Real time) = 0;
		// KeyPressed and keyReleased are called on a keyboard event.
		virtual bool keyPressed(const OIS::KeyEvent &e) = 0;
		virtual bool keyReleased(const OIS::KeyEvent &e)= 0;

		// Responds to mouse input.
		// Mousepressed and mouseReleased are called on a mouse button event.
		virtual bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
		virtual bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
		// MouseMoved is called whenever the mouse is moved.
		virtual bool mouseMoved(const OIS::MouseEvent &e) = 0;

		// Is called everyframe to update a GameState.
		virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt) = 0;

		// A somewhat legacy function. Originally used to update the GUI.
		// Is carried over from old versions for convinence.
		virtual void injectTimestamps(Ogre::Real timeSinceLastFrame) = 0;

		// Changes the current GameState.
		void ChangeState(GameState* state)
		{
			TheApplication.changeState(state);
		}
		// Pushes and Pops GameStates in a vector.
		// Neither function are used however they are included for completeness.
		void pushState(GameState* state)
		{
			TheApplication.pushState(state);
		}
		void popState()
		{
			TheApplication.popState();
		}

	protected:
		// Is protected to ensure each GameState remain singleton.
		GameState() { }

		// Pointers to important data relevant to each GameState.
		Ogre::Root *m_Root;
		Ogre::SceneManager *m_SceneMgr;
		Ogre::Camera *m_Camera;
		Ogre::SceneNode *m_CameraSceneNode;
		Ogre::Viewport *m_ViewPort;
		Ogre::RenderWindow *m_RenderWindow;
		GAME::GameInput *m_Input;
		GAME::GUI *m_GUI;

		// When m_ShutDown becomes true the game will end.
		bool m_ShutDown;
	};
}

#endif