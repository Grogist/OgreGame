#ifndef __GAME_H__
#define __GAME_H__

#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreManualObject.h>
#include <OgreStaticGeometry.h>
#include <OgreParticleSystem.h>

#include <vector>
#include <deque>

#include "tinyxml2.h"

#include "GLunit.h"
#include "GLcommander.h"
#include "GLlocation.h"
#include "GLQuadtree.h"
#include "GLTimerManager.h"
#include "GLFirewall.h"
#include "GlowMaterialListener.h"

namespace GAME
{
	//#define NumberofPointsforVictory 750
	static const int NumberofPointsforVictory = 750;
	//#define WorldSizeX 3500
	//#define WorldSizeZ 3500
	//#define MinimumDistanceSquared 10000.0f // 100
	static const float MinimumDistanceSquared = 10000.0f;

	enum QueuryFlags
	{
		SELECTABLE_MASK = 1<<0,
		WALL_MASK = 1<<1,
		CIRCLE_MASK = 1<<2,
		PROJECTILE_MASK = 1<<3,
		UNIT_MASK = 1<<4,
	};

	class Game
	{
	public:
		Game();
		~Game();

		bool createScene(std::string filename);
		// If no value is given, a value is calculated based on the location's dimension.
		void createLocation(Ogre::Vector3 position, Ogre::Vector2 dimension, char* name, LocationType type, int value = 0);
		void createWall(Ogre::Vector3 position, Ogre::Vector3 scale, char* name, char* meshname = NULL);
		void initSceneManager(Ogre::SceneManager *Manager);

		bool LoadMap(std::string filename);
		bool SaveMap(std::string filename);

		Ogre::SceneManager *getSceneManager(void);

		void update(const Ogre::FrameEvent &evt);

		std::vector<int> getScore(void);
		std::vector<int> getResources(void);

		std::deque<GAME::Location> *getResourceList(void);
		std::deque<GAME::Location> *getObjectiveList(void);
		std::deque<GAME::Location> *getBaseList(void);
		std::vector<GAME::Commander *> *getCommanderList(void);
		std::deque<GAME::Projectile> *getProjectileList(void);

		void getQuadTreeInformation(std::vector<UnitNodes> *units, std::vector<LocationNodes> *locations,
			std::vector<FirewallNodes> *firewalls);

		// The detectionTree must be rebuilt everytime a new unit is added or a unit is destroyed.
		GAME::Quadtree detectionTree;
		void checkCollision(void);
		Ogre::Timer rebuildQuadtreeTimer;

		// fromCommander is the commander that called for buffing/slowing.
		// amount is the percent change.
		// eg. slowUnits(1, 0.5f); = slow all units not from Commander1 by 50%.
		// eg. buffUnits(1, 0.5f); = buff all units from Commander1 by 50%.
		// Can be modified to debuff and increase speed.
		void buffUnits(int fromCommander, float amount); // Buff friendly units.
		bool unbuffUnits(int Timer_ID, int fromCommander);
		void slowUnits(int fromCommander, float amount); // Slow enemy units.
		bool unslowUnits(int Timer_ID, int fromCommander);

		void spawnUnit(void);

		void createProjectile(Ogre::Vector3 position, GAME::UnitController *enemy, float speed, int damage, int commanderNumber);
		bool destroyProjectile(int Timer_ID, int index);

		void createFirewall(Ogre::Vector3 position, Ogre::Quaternion orientation);
		bool destroyFirewall(int Timer_ID, int index);

		Ogre::Vector2 getWorldSize(void);

		void Cleanup(void);

		int numberOfEntities;

	private:
		void Game::setBorderMaterial(GAME::Location *location);

		void checkUnitUnit(QuadtreeNode *node, std::vector<UnitNodes> units);
		void checkUnitLocation(QuadtreeNode *node, std::vector<UnitNodes> units, std::vector<LocationNodes> locations);
		void checkUnitFirewall(QuadtreeNode *node, std::vector<UnitNodes> units, std::vector<FirewallNodes> firewalls);
		//void checkUnitProjectile(QuadtreeNode *node, std::vector<UnitNodes> units, std::vector<ProjectileNodes> projectiles);

		Ogre::Vector2 WorldSize;

		Ogre::Timer updateLocationTimer;
		
		Ogre::Timer collisionTimer;

		Ogre::SceneManager *SceneMgr;
		
		std::vector<GAME::Commander *> commanderList;
		
		std::deque<GAME::Location> resourceList;
		std::deque<GAME::Location> objectiveList;
		// baseList[0] is home base of commanderList[0], 1->1, 2->2, etc.
		// commanderList.size() == baseList.size().
		std::deque<GAME::Location> baseList;
		std::deque<Ogre::SceneNode *> wallList;

		std::deque<GAME::Projectile> projectileList;

		std::deque<GAME::Firewall> firewallList;

		TimerManager<Game> mAbilityTimer;
		TimerManager<Game> mProjectileTimer;
		TimerManager<Game> mFirewallTimer;

	};
}
#endif