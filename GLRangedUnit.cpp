#include "GLRangedUnit.h"

#include "GLapplication.h"
#include "GLPlayState.h"

namespace
{
	const float			RangedUnit_moveSpeed = 120.0f;
	const float			RangedUnit_combatmoveSpeed = 40.0f;
	const unsigned int	RangedUnit_detectionRadii = 500;
	const unsigned int	RangedUnit_combatRadii = 50;
	const int			RangedUnit_health = 80;
	const int			RangedUnit_attack = 4; // Melee damage.
	const int			RangedUnit_projectileattack = 8;
	const int			RangedUnit_defense = 1;
	const float			RangedUnit_evasion = 10.0f;
	const float			RangedUnit_accuracy= 80.0f;
	const float			RangedUnit_attackSpeed = 100.0f;
	const float			RangedUnit_projectileattackSpeed = 8000.0f;
	const float			RangedUnit_projectilemoveSpeed = 650.0f;
}

namespace GAME
{
	RangedUnitController::RangedUnitController() { projectileAttackTimer.reset(); }

	RangedUnitController::~RangedUnitController() {}

	void RangedUnitController::initialize(void)
	{
		UnitController::initialize();

		moveSpeed = RangedUnit_moveSpeed;
		combatMoveSpeed = RangedUnit_combatmoveSpeed;
		detectionRadii = RangedUnit_detectionRadii;
		combatRadii = RangedUnit_combatRadii;
		health = RangedUnit_health;
		maxHealth = RangedUnit_health;
		attack = RangedUnit_attack;
		projectileAttack = RangedUnit_projectileattack;
		defense = RangedUnit_defense;
		evasion = RangedUnit_evasion;
		accuracy = RangedUnit_accuracy;
		attackSpeed = RangedUnit_attackSpeed;
		projectileAttackSpeed = RangedUnit_projectileattackSpeed;
		projectileMoveSpeed = RangedUnit_projectilemoveSpeed;
	}

	void RangedUnitController::update(Ogre::Real time)
	{
		if(combatEnemy)
		{
			unitState = COMBAT_STATE;
			if (oldState != unitState)
			{
				moveSpeed = RangedUnit_moveSpeed;
				int sound = SoundManager::getSingletonPtr()->CreateSound(std::string(COMBAT_SOUND));
				int channel = INVALID_SOUND_CHANNEL;
				SoundManager::getSingletonPtr()->PlaySound(sound, sceneNode, &channel);
			}
			oldState = unitState;
		}
		else if(nearestEnemy)
		{
			unitState = CHASE_STATE;
			if (oldState != unitState)
			{
				int sound = SoundManager::getSingletonPtr()->CreateSound(std::string(CHASE_SOUND));
				int channel = INVALID_SOUND_CHANNEL;
				SoundManager::getSingletonPtr()->PlaySound(sound, sceneNode, &channel);
			}
			oldState = unitState;

			moveSpeed = RangedUnit_combatmoveSpeed;
		}
		else if(!moveList.empty())
		{
			unitState = PATH_STATE;
			if(oldState = CHASE_STATE) moveSpeed = RangedUnit_moveSpeed;
			oldState = unitState;
		}
		//else if (  ) unitState = fleeState;
		else unitState = SLEEP_STATE;

		if(unitState != SLEEP_STATE)
			animationState->addTime(time);

		switch(unitState)
		{
		case SLEEP_STATE:
			HandleSleepState(time);
			break;
		case COMBAT_STATE:
			HandleCombatState(time);
			break;
		case CHASE_STATE:
			HandleChaseState(time);
			break;
		case FLEE_STATE:
			HandleFleeState(time);
			break;
		case PATH_STATE:
			HandlePathState(time);
			break;
		default:
			break;
		}
		
		// FIX / DELETE
		Ogre::Vector3 position = getPosition();
		position.y = 60.0f;
		sceneNode->setPosition(position);

		return;
	}

	bool RangedUnitController::HandleSleepState(Ogre::Real time)
	{
		return true;
	}

	bool RangedUnitController::HandleCombatState(Ogre::Real time)
	{
		if(attackTimer.getMilliseconds() > attackSpeed && combatEnemy)
		{
			attackTimer.reset();
			if(rand()%100 < accuracy) // accuracy% chance to hit.
				combatEnemy->damage(attack);
		}
		offPath = true;
		return true;
	}
	// Doesn't chase, moves away from nearestEnemy
	bool RangedUnitController::HandleChaseState(Ogre::Real time)
	{
		if(nearestEnemy)
		{
			//Ogre::Real move = moveSpeed * time;
			Ogre::Vector3 enemyDirection = nearestEnemy->getPosition() - getPosition();
			//enemyDirection.y = 0;
			//enemyDirection.normalise();
			//sceneNode->translate(enemyDirection * move);
			rotateToDirection(enemyDirection);
			//offPath = true;

			// MAKE BETTER !!!
			if(projectileAttackTimer.getMilliseconds() > projectileAttackSpeed)
			{
				Ogre::Vector3 projectileDirection = nearestEnemy->getPosition() - getPosition();
				projectileDirection.y = 0;
				projectileDirection.normalise();
				Ogre::Vector3 projectilePosition = getPosition() + 50*projectileDirection + Ogre::Vector3(0.0f,50.0f,0.0f);

				GAME::PlayState::getSingletonPtr()->getGame()->createProjectile(projectilePosition, nearestEnemy, projectileMoveSpeed, projectileAttack, commanderNumber);

				projectileAttackTimer.reset();
			}
		}
		return true;
	}

	bool RangedUnitController::HandleFleeState(Ogre::Real time)
	{
		unitState = PATH_STATE;
		return true;
	}
	bool RangedUnitController::HandlePathState(Ogre::Real time)
	{
		if(offPath)
		{
			Direction = moveList.at(0) - this->getPosition();
			Distance = Direction.normalise();
			rotateToDirection(Direction);
			offPath = false;
		}
		Ogre::Real move = moveSpeed * time;
		sceneNode->translate(Direction * move);
		Distance -= move;

		if(Distance <= 0 )
		{
			sceneNode->setPosition(moveList.at(0));
			//sceneNode->translate(0,50.0f,0);
			Distance = 0;
			moveList.pop_front();
			if(moveList.size())
			{
				Direction = moveList.at(0) - getPosition();
				Distance = Direction.normalise();
				rotateToDirection(Direction);
			}
		}
		return true;
	}

	void RangedUnitController::buff(float amount)
	{
		attack += int( (float)attack * amount );
		defense += int( (float)defense * amount );
		if(defense <= 1) defense++;
	}
	void RangedUnitController::unbuff(void)
	{
		attack = RangedUnit_attack;
		defense = RangedUnit_defense;
	}
	void RangedUnitController::slow(float amount)
	{
		moveSpeed *= amount;
		attackSpeed /= amount;
	}
	void RangedUnitController::unslow(void)
	{
		moveSpeed = RangedUnit_moveSpeed;
		attackSpeed = RangedUnit_attackSpeed;
	}

	UnitType RangedUnitController::getUnitType(void)
	{
		return UnitType_Ranged;
	}

	float RangedUnitController::getMoveSpeed(void)
	{
		if(unitState == CHASE_STATE)
			return combatMoveSpeed;
		else
			return moveSpeed;
	}


	Projectile::Projectile(Ogre::SceneNode *node, Ogre::Entity *ent, GAME::UnitController *enemy, float speed, int d) :
		destroyed(false)
	{
		sceneNode = node;
		entity = ent;
		targetEnemy = enemy;
		moveSpeed = speed;
		damage = d;
		existanceTimer.reset();
		animationState = entity->getAnimationState("Spin");
	}
	Projectile::~Projectile() { }

	void Projectile::update(Ogre::Real time)
	{
		// FIX THIS!!!
		//Ogre::Vector3 translationVector = direction * moveSpeed * time;

		animationState->addTime(time);

		// Determine is collision. 2500 => 50 units away.
		if( targetEnemy && getPosition().squaredDistance(targetEnemy->getPosition()) <= 2500 )
		{
			setDestroyed(true);
			targetEnemy->damage(getDamage());
		}
		
		
		float move = moveSpeed * time;

		if(targetEnemy)
		{
			direction = targetEnemy->getPosition() - getPosition();
			direction.y = 0;
			direction.normalise();
		}

		sceneNode->translate(direction*move);

	}

	Ogre::Vector3 Projectile::getPosition(void) { return sceneNode->getPosition(); }

	Ogre::Vector2 Projectile::getDimensions(void)
	{
		Ogre::Vector2 dimensions;
		Ogre::Vector3 boxdimensions = entity->getBoundingBox().getMaximum()-entity->getBoundingBox().getMinimum();
		dimensions.x = boxdimensions.x;
		dimensions.y = boxdimensions.z;
		return dimensions;
	}

	GAME::UnitController *Projectile::getTargetEnemy(void){	return targetEnemy; }

	void Projectile::setTargetEnemy(GAME::UnitController *unit)
	{
		targetEnemy = unit;
	}

	int Projectile::getDamage(void) { return damage; }
	void Projectile::setDamage(int d) { damage = d; }

	void Projectile::destroy(void)
	{
		sceneNode->detachAllObjects();
		sceneNode->removeAndDestroyAllChildren();
		GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->destroyEntity(entity);
		GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->getRootSceneNode()->removeAndDestroyChild(sceneNode->getName());

		destroyed = true;
	}

	bool Projectile::isDestroyed(void) { return destroyed; }

	void Projectile::setDestroyed(bool set) { destroyed = set; }

	void Projectile::rotateToDirection(Ogre::Vector3 direction)
	{

	}
}