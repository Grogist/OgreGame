#include "GLNormalUnit.h"

#include "GLapplication.h"

namespace
{
	const float			NormalUnit_moveSpeed = 140.0f;
	const unsigned int	NormalUnit_detectionRadii = 200;
	const unsigned int	NormalUnit_combatRadii = 50;
	const int			NormalUnit_health = 100;
	const int			NormalUnit_attack = 10;
	const int			NormalUnit_defense = 2;
	const float			NormalUnit_evasion = 15.0f;
	const float			NormalUnit_accuracy= 85.0f;
	const float			NormalUnit_attackSpeed = 100.0f;
}

namespace GAME
{
	NormalUnitController::NormalUnitController() {}

	NormalUnitController::~NormalUnitController() {}

	void NormalUnitController::initialize(void)
	{
		UnitController::initialize();

		moveSpeed = NormalUnit_moveSpeed;
		detectionRadii = NormalUnit_detectionRadii;
		combatRadii = NormalUnit_combatRadii;
		health = NormalUnit_health;
		maxHealth = NormalUnit_health;
		attack = NormalUnit_attack;
		defense = NormalUnit_defense;
		evasion = NormalUnit_evasion;
		accuracy = NormalUnit_accuracy;
		attackSpeed = NormalUnit_attackSpeed;
	}

	void NormalUnitController::update(Ogre::Real time)
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
		case FLEE_STATE :
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

	bool NormalUnitController::HandleSleepState(Ogre::Real time)
	{
		return true;
	}

	bool NormalUnitController::HandleCombatState(Ogre::Real time)
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
	bool NormalUnitController::HandleChaseState(Ogre::Real time)
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

	bool NormalUnitController::HandleFleeState(Ogre::Real time)
	{
		unitState = PATH_STATE;
		return true;
	}
	bool NormalUnitController::HandlePathState(Ogre::Real time)
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

	void NormalUnitController::buff(float amount)
	{
		attack += int( (float)attack * amount );
		defense += int( (float)defense * amount );
		if(defense <= 1) defense++;
	}
	void NormalUnitController::unbuff(void)
	{
		attack = NormalUnit_attack;
		defense = NormalUnit_defense;
	}
	void NormalUnitController::slow(float amount)
	{
		moveSpeed *= amount;
		attackSpeed /= amount;
	}
	void NormalUnitController::unslow(void)
	{
		moveSpeed = NormalUnit_moveSpeed;
		attackSpeed = NormalUnit_attackSpeed;
	}

	UnitType NormalUnitController::getUnitType(void)
	{
		return UnitType_Normal;
	}
}