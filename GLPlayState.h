/***********************************************************************

File: GLPlayState.h
Author: Gregory Libera

Description: PlayState represents the "game" part of the Game. It inherits
from GameState and is one of the two game states. The game enters
PlayState when the user loads a map in MenuState.
Uncommented funtions are described in GLGameState.h

***********************************************************************/

#ifndef __PlayState_H__
#define __PlayState_H__

#include <Ogre.h>

#include "GLGameState.h"
#include "GLActionList.h"
#include "GLunit.h"
#include "GLParticleSystemManager.h"

namespace GAME
{
	class PlayState : public GameState
	{
	public:
		~PlayState();
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

		static PlayState* getSingletonPtr() { return &m_PlayState; }

		// Returns the position of the first intersect of ray cast by the 
		//  mouse cursor into the game world.
		Ogre::Vector3 getRayPosition(void);

		// Returns the current action type. (See GLActionList.h);
		ActionType getActionType(void);
		// Sets the current action type.
		void setActionType(ActionType actionType);

		// Sets the currently selected unit. The selected unit is obtained
		//  by clicking on it in the game world.
		void setSelectedUnit(GAME::UnitController *unit);
		// Returns the selected unit.
		GAME::UnitController *getSelectedUnit(void);
		// Checks if the parameter and the pointer reference of selectedunit,
		//  targetunit, or the target of a projectile are the same.
		// If they are they must be changed to NULL.
		void checkUnitPointerConflicts(GAME::UnitController *unit);

		// Places the firewallBoxSceneNode into the scene graph.
		// Is called whenever the action changes to firewall action.
		void firewallAction(void);

		// Returns a pointer to the current Game.
		GAME::Game *getGame(void);
		// Returns a pointer to the current Minimap.
		GAME::Minimap *getMinimap(void);
		// Returns a pointer to the current Scene Manager.
		Ogre::SceneManager *getSceneManager(void);

		// Pauses/UnPauses the Game.
		void setPaused(bool pause);
		// Returns the pause state.
		bool getPaused(void);
		// Thes the game's winner
		void setGameWonBy(int victor);
		// Returns the Commander that has won a game. If returns 0
		//  no Commander has won the game yet.
		int getGameWonBy(void);

	protected:
		PlayState();

	private:
		static PlayState m_PlayState;

		// The current Game.
		GAME::Game m_TheGame;
		// The current Minimap
		GAME::Minimap m_Minimap;
		// Timer to update the minimap.
		Ogre::Timer m_MiniMapUpdateTimer;

		GAME::ParticleSystemManager m_ParticleSystemManager;

		// Cause the camera to move when user moves the mouse to the edge of the screen,
		//  or when the appropriate key is pressed.
		//  modifier should be +1 or -1 to change the direction of movement.
		void scrollVertical(int modifier, Ogre::Real time);
		void scrollHorizontal(int modifier, Ogre::Real time);

		// Shows the detection and combat circles of a unit.
		void showCircles(bool show);

		GAME::UnitController *findUnitControllerFromSceneNode( Ogre::SceneNode *sceneNode );
		bool findLocationFromSceneNode( Ogre::SceneNode *sceneNode );

		// Is Left/Right/Middle mouse down?
		bool m_LMouseDown;
		bool m_RMouseDown;
		bool m_MMouseDown;

		// Camera scroll speed.
		float scrollSpeed;
		
		bool gamePaused;
		// GameWonBy corresponds to a commander number. 0 means the game is not won yet.
		int gameWonBy;

		Ogre::RaySceneQuery *m_RaySceneQuery;
		
		Ogre::Vector3 m_RayPosition;

		// Reference to the Entity and SceneNode of the arrow that appears over a selected unit.
		Ogre::Entity *m_Arrow;
		Ogre::SceneNode *m_ArrowSceneNode;

		Ogre::ManualObject *m_FirewallBox;
		Ogre::SceneNode *m_FirewallBoxSceneNode;
		// The orientation of the firewallbox before a mouse click.
		Ogre::Quaternion orientationBeforeRelease;

		// Creates the firewall box manual object.
		void createBox(Ogre::ManualObject *box);

		// The current ActionType
		ActionType m_ActionType;

		// CurrentObject is the unit SceneNode that is current being hovered over by the mouse.
		Ogre::SceneNode *m_CurrentObject;
		// TargetedUnit is used for actions.
		GAME::UnitController *m_TargetedUnit;
		// SelectedUnit is used for GUI display.
		GAME::UnitController *m_SelectedUnit;

		// Used for selecting locations.
		LocationType m_StoredLocationType;
		// The index in the appropriate location list of the stored location.
		int m_StoredIndex;
		int m_StoredBaseIndex;		

		// The windows used to show the commander scores and resources.
		std::vector<CEGUI::Window*> m_CommanderScores;
		std::vector<CEGUI::Window*> m_CommanderResources;
		// The colour of each commander.
		std::vector<std::string>	m_CommanderTextColours;

		// Pointers to GUI windows.
		CEGUI::Window *xCoordinate;
		CEGUI::Window *yCoordinate;
		CEGUI::Window *fps;
		CEGUI::Window *popup;
		CEGUI::Window *priority;		
		CEGUI::Window *unitHealth;
		CEGUI::Window *unitAttack;
		CEGUI::Window *unitDefense;
		CEGUI::Window *unitAttackSpeed;
		CEGUI::Window *unitMoveSpeed;
		CEGUI::Window *unitAccuracy;
		CEGUI::Window *unitEvasion;

		// Used for framerate calculations.
		int frame;
		Ogre::Timer fpsTimer;
		float fpsSamples[64];

		// Callback functions for the playstate GUI buttons.
		bool quitfunction(const CEGUI::EventArgs &e);
		bool unloadfunction(const CEGUI::EventArgs &e);
		bool leftarrowfunction(const CEGUI::EventArgs &e);
		bool rightarrowfunction(const CEGUI::EventArgs &e);
		bool fireballfunction(const CEGUI::EventArgs &e);
		bool slowfunction(const CEGUI::EventArgs &e);
		bool bufffunction(const CEGUI::EventArgs &e);
		bool spawnfunction(const CEGUI::EventArgs &e);
		bool teleportfunction(const CEGUI::EventArgs &e);
		bool confusefunction(const CEGUI::EventArgs &e);
		bool stealthfunction(const CEGUI::EventArgs &e);
		bool firewallfunction(const CEGUI::EventArgs &e);
		bool defaultactionfunction(const CEGUI::EventArgs &e);
		bool minimapclickfunction(const CEGUI::EventArgs &e);

		// Is called whenever an action is activated.
		void actionActivated(ActionType actionType);

		// Is whether of not an action has been activated and it's cooldown has not expired.
		bool fireballActivated, slowActivated, buffActivated, spawnActivated, teleportActivated, confusedActivated,
			stealthActivated, firewallActivated;

		// Shows the priority window, with it's relavent information.
		void popupWindow(CEGUI::Point mousePosition, LocationType type, int index, bool enable);

		// Shows the windows displaying X,Z position of the m_RayPosition, and the current FPS.
		void showDeveloper(void);
	};
}
#endif