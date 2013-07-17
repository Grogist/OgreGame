#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include "GLunit.h"
#include "GLFastUnit.h"
#include "GLNormalUnit.h"
#include "GLSlowUnit.h"
#include "GLRangedUnit.h"
#include "GLlocation.h"
#include "GLPath.h"
#include "GLTimerManager.h"

#include <OgreTimer.h>

#include <vector>
#include <tuple>

namespace GAME
{
	enum UnitColour
	{
		RED,
		GREEN,
		BLUE,
		DARKBLUE
	};

	//#define DEFAULT_MaxPriority			10
	//#define DEFAULT_ValuePriorityAdjust	3

	static const int DEFAULT_MaxPriority = 10;
	static const int DEFAULT_ValuePriorityAdjust = 3;

	class Commander
	{
	public:
		Commander();
		~Commander();

		std::deque<GAME::UnitController *> unitList;

		void runAI(Ogre::Real time);

		void addUnit(GAME::UnitController *newUnit);
		// Returns the cost of the created unit.
		void createUnit();
		void createUnit(Ogre::Vector3 position, UnitType type);
		// Sets the location lists and their initial priorities.
		// RENAME INITIALIZE
		void setPointers(std::deque<GAME::Location> *resource,
			std::deque<GAME::Location> *objective, std::deque<GAME::Location> *base,
			std::deque<Ogre::SceneNode *> *wall);

		int getScore(void);
		void addToScore(int add);

		// Sets a pointer to the homeBase and provides the commander
		//  with it's commanderNumber
		void setHomeBase(GAME::Location* base, int number);
		void setUnitColour(UnitColour colour);
		UnitColour getUnitColour(void);
		
		int getResources(void);
		void addToResources(int add);

		int getLocationPriority(int index, LocationType type);
		int getCommanderNumber(void);
		int getBaseHealth(void);
		void damageBase(int damage);

		void modifyLocationPriority(int index, int value, LocationType type);
		
		GAME::Path path;

		void confuseUnit(GAME::UnitController *unit, int confusedBy);
		bool unconfuseUnit(int Timer_ID, int unitIndex);
		void hideUnit(GAME::UnitController *unit);
		bool unhideUnit(int Timer_ID, int unitIndex);
		void teleportUnit(GAME::UnitController *unit, Ogre::Vector3 destination);

		void Cleanup(void);

	private:
		Ogre::Timer genTimer;
		TimerManager<Commander> mTimer;

		std::deque<Ogre::SceneNode *> *wallList;

		// First: pointer to a location in Game::resourceList, Second: priority
		// Base Priority ranges from 0 to 10. The higher the priority the more likely
		//  the Commander is to capture and hold a location.
		std::deque<std::pair<GAME::Location *, int>> resourceList;
		std::deque<std::pair<GAME::Location *, int>> objectiveList;
		std::deque<std::pair<GAME::Location *, int>> baseList;
		// List of all locations not currently controlled, or being targeted for control.
		// I SHOULD SORT THIS BY PRIORITY!
		std::deque<std::pair<GAME::Location *, int>> availableList;
		// availableList.size() == isTargeted.size()
		std::vector<bool> isTargeted;

		// THIS SHOULD BE A CONST.
		//int homeBase;
		enum UnitColour colour;
		GAME::Location* homeBase;

		int commanderNumber;
		int commanderScore;
		int commanderResources;

		int baseHealth;

		UnitType nextUnit;
		int nextUnitCost;
	};
}
#endif