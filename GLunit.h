/***********************************************************************

File: GLunit.h
Author: Gregory Libera

Description: Unit is an abstract class which describes all of the common
elements of the different unit typea.

***********************************************************************/

#ifndef __Unit_H__
#define __Unit_H__

#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreNode.h>
#include <OgreRay.h>
#include <OgreTimer.h>

#include "GLlocation.h"

#include <deque>

namespace GAME
{
	// Each type of unit of stated in UnitType
	enum UnitType
	{
		UnitType_Fast,
		UnitType_Normal,
		UnitType_Slow,
		UnitType_Ranged,
	};

	static const int	DEFAULT_DetectionRadii = 200;
	static const int	DEFAULT_CombatRadii = 50;
	static const float	DEFAULT_MoveSpeed = 140.0f;

	// Location of chase and combat sounds.
	static const char *CHASE_SOUND  = "../Sound/mug-down-1.wav";
	static const char *COMBAT_SOUND = "../Sound/glass-clink-2.wav";

	// The different states that a unit can be in.
	enum UnitState
	{
		SLEEP_STATE,
		COMBAT_STATE,
		CHASE_STATE,
		FLEE_STATE,
		PATH_STATE,
	};

	class UnitController
	{
	public:
		UnitController();
		~UnitController();

		// References to the entity and scene node for a unit.
		Ogre::Entity *entity;
		Ogre::SceneNode *sceneNode;

		// Combat circle is the visual representation of the radius
		//  that a unit will engage a hostile unit in combat.
		Ogre::ManualObject *combatCircle;
		Ogre::SceneNode *combatCircleNode;
		// Detection circle is the visual representation of the radius
		//  that a unit will attempt to chase a hostile unit.
		Ogre::ManualObject *detectionCircle;
		Ogre::SceneNode *detectionCircleNode;

		virtual void initialize(void);

		virtual void update(Ogre::Real time) = 0;		

		// Sets the destination of a unit. This clears moveList.
		void setDestination(Ogre::Vector3 destination);
		// Adds another Vector3 to moveList.
		void addDestination(Ogre::Vector3 destination);
		// Sets the path for a unit.
		void setPath(std::deque<Ogre::Vector3> path);
		// If a unit goes off of its path (to pursue and enemy unit),
		//  it is then off its path.
		void setOffPath(bool set);

		// Returns a units current position.
		Ogre::Vector3 getPosition(void);

		// Alerts Unit to an enemy unit within its detection range.
		void enemyUnitInRange(UnitController *enemy);

		// Sets unit in combat with enemy.
		void inCombat(UnitController *enemy);

		// Sets enemy pointers to NULL.
		void cleanUpEnemyPointers(void);

		// Returns the units current state.
		UnitState getUnitState(void);

		// Causes damage to a unit.
		void damage(int amount);

		// Kills the unit
		void kill(void);

		// Gets/Set various stats of a unit.
		int				getHealth(void);
		int				getMaxHealth(void);
		int				getAttack(void);
		void			setAttack(int a);
		int				getDefense(void);
		void			setDefense(int a);
		float			getEvasion(void);
		float			getAccuracy(void);
		virtual float	getAttackSpeed(void);
		virtual float	getMoveSpeed(void);
		virtual void	setMoveSpeed(float speed);

		// Returns the unit's commander's number. Indexed from 1.
		int getCommanderNumber(void);
		// Sets the unit's commander's number. Indexed from 1.
		void setCommanderNumber(int number);

		// Gets/Sets the commander a unit is confused by.
		int		getConfusedBy(void);
		void	setConfusedBy(int set);
		// Gets/Sets if a unit is hidden.
		bool	isHidden(void);
		void	setHidden(bool set);

		// The radius of the detectionCircle and combatCircle.
		unsigned int detectionRadii;
		unsigned int combatRadii;

		virtual void buff(float amount) = 0;
		virtual void unbuff(void) = 0;
		virtual void slow(float amount) = 0;
		virtual void unslow(void) = 0;

		// Returns the unit's type.
		virtual UnitType getUnitType(void) = 0;

		// targetLocation is the location a unit is either
		//  currently occupying or is moving toward.
		// The first element is a pointer to a location.
		// The second element is the priority of that location.
		std::pair<GAME::Location *, int> targetLocation;

	protected:
		
		enum UnitState unitState;
		enum UnitState oldState;

		// Rotates a unit to direction.
		void rotateToDirection(Ogre::Vector3 direction);
		bool Walkable(Ogre::Vector3 from, Ogre::Vector3 to);

		// The direction that a unit is moving.
		Ogre::Vector3 Direction;
		// The distance until a unit reaches its destination.
		float Distance;

		// The list of locations on a path the leads to a destination.
		std::deque<Ogre::Vector3> moveList;

		UnitController *nearestEnemy;
		UnitController *combatEnemy;

		// The max possible health of a unit.
		int maxHealth;
		// A units stats.
		int health;
		int attack;
		int defense;
		float evasion;
		float accuracy;
		// The attack speed is in milliseconds.
		float attackSpeed;
		float moveSpeed;
		Ogre::Timer attackTimer;
		// Is true if a unit is fleeing combat.
		bool flee;

		//The unit's commander's number. Indexed from 1.
		int commanderNumber;

		// Is true if a unit moves from its path.
		bool offPath;
		// Is true if a unit is in combat.
		bool isInCombat;

		// ConfusedBy is the commander's number that is confusing
		//  a unit. 0 means a unit is not confused.
		int confusedBy; 
		// Whether or not a unit is hidden.
		bool hidden;

		// Is used for path smoothing. Determines if a future node
		//  can be moved to a previous node.
		Ogre::Ray ray;
		Ogre::RaySceneQuery *query;

		Ogre::AnimationState *animationState;		
	};
}
#endif