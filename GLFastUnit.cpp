#include "GLFastUnit.h"

#include "GLapplication.h"

namespace
{
	const float			FastUnit_moveSpeed = 180.0f;
	const unsigned int	FastUnit_detectionRadii = 250;
	const unsigned int	FastUnit_combatRadii = 50;
	const int			FastUnit_health = 80;
	const int			FastUnit_attack = 6;
	const int			FastUnit_defense = 1;
	const float			FastUnit_evasion = 20.0f;
	const float			FastUnit_accuracy= 90.0f;
	const float			FastUnit_attackSpeed = 50.0f;
}

namespace GAME
{
	FastUnitController::FastUnitController() {}

	FastUnitController::~FastUnitController() {}

	void FastUnitController::initialize(void)
	{
		UnitController::initialize();

		moveSpeed = FastUnit_moveSpeed;
		detectionRadii = FastUnit_detectionRadii;
		combatRadii = FastUnit_combatRadii;
		health = FastUnit_health;
		maxHealth = FastUnit_health;
		attack = FastUnit_attack;
		defense = FastUnit_defense;
		evasion = FastUnit_evasion;
		accuracy = FastUnit_accuracy;
		attackSpeed = FastUnit_attackSpeed;
	}

	void FastUnitController::update(Ogre::Real time)
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

	bool FastUnitController::HandleSleepState(Ogre::Real time)
	{
		return true;
	}

	bool FastUnitController::HandleCombatState(Ogre::Real time)
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
	bool FastUnitController::HandleChaseState(Ogre::Real time)
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

	bool FastUnitController::HandleFleeState(Ogre::Real time)
	{
		unitState = PATH_STATE;
		return true;
	}
	bool FastUnitController::HandlePathState(Ogre::Real time)
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

	void FastUnitController::buff(float amount)
	{
		attack += int( (float)attack * amount );
		defense += int( (float)defense * amount );
		if(defense <= 1) defense++;
	}
	void FastUnitController::unbuff(void)
	{
		attack = FastUnit_attack;
		defense = FastUnit_defense;
	}
	void FastUnitController::slow(float amount)
	{
		moveSpeed *= amount;
		attackSpeed /= amount;
	}
	void FastUnitController::unslow(void)
	{
		moveSpeed = FastUnit_moveSpeed;
		attackSpeed = FastUnit_attackSpeed;
	}

	UnitType FastUnitController::getUnitType(void)
	{
		return UnitType_Fast;
	}
}