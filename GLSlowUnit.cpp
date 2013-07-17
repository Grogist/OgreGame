#include "GLSlowUnit.h"

#include "GLapplication.h"

namespace
{
	const float			SlowUnit_moveSpeed = 100.0f;
	const unsigned int	SlowUnit_detectionRadii = 150;
	const unsigned int	SlowUnit_combatRadii = 50;
	const int			SlowUnit_health = 120;
	const int			SlowUnit_attack = 18;
	const int			SlowUnit_defense = 6;
	const float			SlowUnit_evasion = 10.0f;
	const float			SlowUnit_accuracy= 80.0f;
	const float			SlowUnit_attackSpeed = 200.0f;
}

namespace GAME
{
	SlowUnitController::SlowUnitController() {}

	SlowUnitController::~SlowUnitController() {}

	void SlowUnitController::initialize(void)
	{
		UnitController::initialize();

		moveSpeed = SlowUnit_moveSpeed;
		detectionRadii = SlowUnit_detectionRadii;
		combatRadii = SlowUnit_combatRadii;
		health = SlowUnit_health;
		maxHealth = SlowUnit_health;
		attack = SlowUnit_attack;
		defense = SlowUnit_defense;
		evasion = SlowUnit_evasion;
		accuracy = SlowUnit_accuracy;
		attackSpeed = SlowUnit_attackSpeed;
	}

	void SlowUnitController::update(Ogre::Real time)
	{
		if(combatEnemy)
		{
			unitState = COMBAT_STATE;
			if (oldState != unitState)
			{
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
		}
		else if(!moveList.empty()) unitState = PATH_STATE;
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

	bool SlowUnitController::HandleSleepState(Ogre::Real time)
	{
		return true;
	}

	bool SlowUnitController::HandleCombatState(Ogre::Real time)
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
	bool SlowUnitController::HandleChaseState(Ogre::Real time)
	{
		Ogre::Real move = moveSpeed * time;
		Ogre::Vector3 enemyDirection = nearestEnemy->sceneNode->getPosition() - this->getPosition();
		enemyDirection.y = 0;
		enemyDirection.normalise();
		sceneNode->translate(enemyDirection * move);
		rotateToDirection(enemyDirection);
		offPath = true;

		return true;
	}

	bool SlowUnitController::HandleFleeState(Ogre::Real time)
	{
		unitState = PATH_STATE;
		return true;
	}
	bool SlowUnitController::HandlePathState(Ogre::Real time)
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

	void SlowUnitController::buff(float amount)
	{
		attack += int( (float)attack * amount );
		defense += int( (float)defense * amount );
		if(defense <= 1) defense++;
	}
	void SlowUnitController::unbuff(void)
	{
		attack = SlowUnit_attack;
		defense = SlowUnit_defense;
	}
	void SlowUnitController::slow(float amount)
	{
		moveSpeed *= amount;
		attackSpeed /= amount;
	}
	void SlowUnitController::unslow(void)
	{
		moveSpeed = SlowUnit_moveSpeed;
		attackSpeed = SlowUnit_attackSpeed;
	}

	UnitType SlowUnitController::getUnitType(void)
	{
		return UnitType_Slow;
	}
}