/***********************************************************************

File: GLMenuState.h
Author: Gregory Libera

Description: MenuState represents the main menu of the Game. It inherits
from GameState and is one of the two game states. When the game is
loaded this is the state that it first enters.
Uncommented funtions are described in GLGameState.h

***********************************************************************/

#ifndef __MenuState_H__
#define __MenuState_H__

#include <Ogre.h>
#include <boost/filesystem.hpp>

#include "GLGameState.h"

namespace GAME
{
	class MenuState : public GameState
	{
	public:
		~MenuState() {};
		void Init(void);
		void Cleanup(void);

		void Pause(void);
		void Resume(void);

		void capture(Ogre::Real time);
		bool keyPressed(const OIS::KeyEvent &e);
		bool keyReleased(const OIS::KeyEvent &e);

		bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseMoved(const OIS::MouseEvent &e);

		bool frameRenderingQueued(const Ogre::FrameEvent &evt);

		void injectTimestamps(Ogre::Real timeSinceLastFrame);

		// Returns a pointer to the singleton MenuState;
		static MenuState* getSingletonPtr() { return &m_MenuState; }

		// A string containing the filename of the currently selected map.
		// Is used when loading a map in GAME::Game.
		std::string m_MapName;
		// A list containing the names of all available maps.
		std::vector<std::string> m_MapList;
		// A list containing the description of all available maps.
		std::vector<std::string> m_DescriptionList;
		// A list containing a reference to the CEGUI Imageset for all available maps.
		std::vector<CEGUI::Imageset*> m_ImageList;

		// The current colour of the player.
		GAME::UnitColour m_PlayerColour;

	protected:
		MenuState() {};

	private:
		// m_MenuState is the singleton instance of MenuState;
		static MenuState m_MenuState;

		// These functions are called when the user interacts with specific
		//  parts of the GUI.
		// Causes the game to ent.
		bool quitfunction(const CEGUI::EventArgs &e);
		// Attempts to change the GameState to PlayState.
		bool loadfunction(const CEGUI::EventArgs &e);
		// Switches the menu to the load map menu.
		bool loadmenufunction(const CEGUI::EventArgs &e);
		// Switches the menu to the credits menu.
		bool creditsfunction(const CEGUI::EventArgs &e);
		// Switches the menu to the main menu.
		bool returnfunction(const CEGUI::EventArgs &e);
		// Is called when the user interacts with the map list in the load map menu.
		bool maplistselectionfunction(const CEGUI::EventArgs &e);
		// These are called when the user interacts with the different colour
		//  selection windows.
		bool bluefunction(const CEGUI::EventArgs &e);
		bool redfunction(const CEGUI::EventArgs &e);
		bool greenfunction(const CEGUI::EventArgs &e);
		bool darkbluefunction(const CEGUI::EventArgs &e);

		// Creates the available list map list.
		void createMapList(void);
	};
}
#endif