#ifndef __RangedUnit_H__
#define __RangedUnit_H__

#include "GLunit.h"

namespace GAME
{
	class RangedUnitController : public UnitController
	{
	public:
		RangedUnitController();
		~RangedUnitController();

		void initialize();

		void update(Ogre::Real time);

		void buff(float amount);
		void unbuff(void);
		void slow(float amount);
		void unslow(void);

		UnitType getUnitType(void);

		float getMoveSpeed(void);

	private:

		float combatMoveSpeed;
		float projectileMoveSpeed;
		int	  projectileAttack;
		float projectileAttackSpeed;
		Ogre::Timer projectileAttackTimer;

		bool HandleSleepState(Ogre::Real time);
		bool HandleCombatState(Ogre::Real time);
		bool HandleChaseState(Ogre::Real time);
		bool HandleFleeState(Ogre::Real time);
		bool HandlePathState(Ogre::Real time);
	};

	class Projectile
	{
	public:
		Projectile(Ogre::SceneNode *node, Ogre::Entity *ent, GAME::UnitController *enemy, float speed, int d);
		~Projectile();

		void update(Ogre::Real time);

		Ogre::Vector3 getPosition(void);
		Ogre::Vector2 getDimensions(void);
		GAME::UnitController *getTargetEnemy(void);
		void setTargetEnemy(GAME::UnitController *unit);

		int getDamage(void);
		void setDamage(int damage);
		void destroy();
		bool isDestroyed(void);
		void setDestroyed(bool set);

	private:
		Ogre::SceneNode *sceneNode;
		Ogre::Entity *entity;
		Ogre::Timer existanceTimer;

		int damage;
		float moveSpeed;
		float initialAngle;
		GAME::UnitController *targetEnemy;
		Ogre::Vector3 direction;
		bool destroyed;
		void rotateToDirection(Ogre::Vector3 direction);

		Ogre::AnimationState *animationState;

	};
}
#endif