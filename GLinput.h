/*********************

GameInput controls all user input using
a keybord and mouse.

*********************/

#ifndef __Input_H__
#define __Input_H__

#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <OgreRenderWindow.h>
#include <OgreCamera.h>
#include <OgreSceneQuery.h>
#include <OgreManualObject.h>

#include "GLActionList.h"
#include "GLunit.h"

#include <CEGUI.h>

namespace GAME
{
	class GameInput : public OIS::KeyListener, public OIS::MouseListener
	{
	public:
		~GameInput(void);

		void initialize(Ogre::RenderWindow *window);
		void setWindowExtents(int width, int height);
		void setWindowClosed(void);

		void capture(Ogre::Real time);

		static GameInput *getSingletonPtr(void);

		bool isKeyDown(OIS::KeyCode key);

	private:
		GameInput(void);
		GameInput(const GameInput&) { }
		GameInput & operator = (const GameInput&);

		bool keyPressed(const OIS::KeyEvent &e);
		bool keyReleased(const OIS::KeyEvent &e);

		bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseMoved(const OIS::MouseEvent &e);

		OIS::InputManager *mInputManager;
		OIS::Mouse *mMouse;
		OIS::Keyboard *mKeyboard;

		static GameInput mInput;
	};
}
#endif