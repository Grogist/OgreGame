#include "GLinput.h"

//TEMP
#include <OgreLog.h>
#include <OgreLogManager.h>

#include "GLapplication.h"
#include "GLGameState.h"

CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
	switch(buttonID)
	{
	case OIS::MB_Left:
		return CEGUI::LeftButton;
	case OIS::MB_Right:
		return CEGUI::RightButton;
	case OIS::MB_Middle:
		return CEGUI::MiddleButton;
	default:
		return CEGUI::LeftButton;
	}
}

namespace GAME
{
	GameInput GameInput::mInput;

	GameInput::GameInput() : mMouse(NULL), mKeyboard(NULL), mInputManager(NULL)
	{ }

	GameInput::~GameInput()
	{
		if(mInputManager)
		{
			if(mMouse)
			{
				mInputManager->destroyInputObject(mMouse);
			}
			if(mKeyboard)
			{
				mInputManager->destroyInputObject(mKeyboard);
			}
		}

		mInputManager->destroyInputSystem(mInputManager);
		mInputManager = 0;
	}

	void GameInput::initialize(Ogre::RenderWindow *window)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");

		if(!mInputManager)
		{
			OIS::ParamList pl;
			size_t windowHnd = 0;
			std::ostringstream windowHndStr;

			window->getCustomAttribute("WINDOW", &windowHnd);

			windowHndStr << (unsigned int) windowHnd;
			pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

			mInputManager = OIS::InputManager::createInputSystem(pl);

			mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, false ));
			mKeyboard->setEventCallback(this);
			mKeyboard->setBuffered(true);
			mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, false ));
			mMouse->setEventCallback(this);
			mMouse->setBuffered(true);

			// Get Window Size
			unsigned int width, height, depth;
			int left, top;
			window->getMetrics(width, height, depth, left, top);

			this->setWindowExtents(width,height);

			// INCREDIBLY HACKISH!!!
			OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
			mutableMouseState.X.abs = 1;
			mutableMouseState.Y.abs = 1;
		}

		//inputCamera = TheApplication.getCamera();
		//inputVP = TheApplication.getViewport();
		Ogre::LogManager::getSingletonPtr()->logMessage("*** Done Initializing OIS ***");
	}

	void GameInput::setWindowExtents(int width, int height)
	{
		mMouse->getMouseState().width = width;
		mMouse->getMouseState().height = height;
	}

	void GameInput::setWindowClosed(void)
	{
		if (mInputManager)
		{
			mInputManager->destroyInputObject(mMouse);
			mInputManager->destroyInputObject(mKeyboard);

			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
			Ogre::LogManager::getSingletonPtr()->logMessage("*** Closing Window ***");
		}
	}
	
	void GameInput::capture(Ogre::Real time)
	{
		if(mMouse) mMouse->capture();

		if(mKeyboard) mKeyboard->capture();

		TheApplication.getGameState()->capture(time);
	}

	GameInput *GameInput::getSingletonPtr(void)
	{
		return &mInput;
	}

	bool GameInput::isKeyDown(OIS::KeyCode key)
	{
		return mKeyboard->isKeyDown(key);
	}

	bool GameInput::keyPressed(const OIS::KeyEvent &e)
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectKeyDown(e.key);
		sys.injectChar(e.text);

		TheApplication.getGameState()->keyPressed(e);

		return true;
	}

	bool GameInput::keyReleased(const OIS::KeyEvent &e)
	{
		CEGUI::System::getSingleton().injectKeyUp(e.key);

		TheApplication.getGameState()->keyReleased(e);

		return true;
	}

	bool GameInput::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));

		TheApplication.getGameState()->mousePressed(e, id);

		return true;
	}

	bool GameInput::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));

		TheApplication.getGameState()->mouseReleased(e, id);

		// RELIC FIX		
		if(id == OIS::MB_Middle)
		{
			CEGUI::Point mousePosition = CEGUI::MouseCursor::getSingleton().getPosition();
			OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());	
			CEGUI::MouseCursor::getSingleton().show();			

			// Set OIS mouse position = CEGUI mouse postion
			// HACKISH!!!
			mutableMouseState.X.abs = int(mousePosition.d_x);
			mutableMouseState.Y.abs = int(mousePosition.d_y);
		}

		return true;
	}

	bool GameInput::mouseMoved( const OIS::MouseEvent &e )
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();

		TheApplication.getGameState()->mouseMoved(e);

		return true;
	}
}