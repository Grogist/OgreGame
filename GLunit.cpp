#include "GLunit.h"

#include "GLapplication.h"
#include "GLPlayState.h"

namespace GAME
{
	UnitController::UnitController() : moveSpeed(DEFAULT_MoveSpeed), health(100), attack(10), defense(2),
		evasion(20.0f), accuracy(80.0f), attackSpeed(100.0f), detectionRadii(DEFAULT_DetectionRadii),
		combatRadii(DEFAULT_CombatRadii), nearestEnemy(NULL), combatEnemy(NULL), flee(false), offPath(false),
		unitState(SLEEP_STATE), oldState(SLEEP_STATE), confusedBy(0), hidden(false)
	{
		Direction = Ogre::Vector3::ZERO;
		attackTimer.reset();
		targetLocation.first = NULL;
		targetLocation.second = 0;
	}

	UnitController::~UnitController() {}

	void UnitController::initialize(void)
	{
		sceneNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Math::PI/2));
		moveList.push_back(sceneNode->getPosition());
		Direction = moveList.at(0) - sceneNode->getPosition();
		Distance = Direction.normalise();

		if( entity->getMesh()->getName() == Ogre::String("penguin.mesh") )
			animationState = entity->getAnimationState("amuse");
		else if( entity->getMesh()->getName() == Ogre::String("robot.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("ninja.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("jaiqua.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("fish.mesh") )
			animationState = entity->getAnimationState("swim");
		else if( entity->getMesh()->getName() == Ogre::String("Sinbad.mesh") )
			animationState = entity->getAnimationState("RunBase");
		else if( entity->getMesh()->getName() == Ogre::String("Cube.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("Pyramid.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("Dodecahedron.mesh") )
			animationState = entity->getAnimationState("Walk");
		else if( entity->getMesh()->getName() == Ogre::String("Octahedron.mesh") )
			animationState = entity->getAnimationState("Walk");

		animationState->setLoop(true);
		animationState->setEnabled(true);
	}

	void UnitController::setDestination(Ogre::Vector3 destination)
	{
		moveList.clear();
		destination.y = getPosition().y;
		moveList.push_back(destination);
		Direction = destination - getPosition();
		Distance = Direction.normalise();
		rotateToDirection(Direction);
	}

	void UnitController::addDestination(Ogre::Vector3 destination)
	{
		destination.y = getPosition().y;
		moveList.push_back(destination);
	}

	// MAKE BETTER
	bool UnitController::Walkable(Ogre::Vector3 from, Ogre::Vector3 to)
	{
		query = GAME::PlayState::getSingletonPtr()->getSceneManager()->createRayQuery(Ogre::Ray());

		ray.setOrigin(from);
		ray.setDirection(to-from);
		query->setRay(ray);
		query->setQueryMask(WALL_MASK);
		query->setSortByDistance(true);
		Ogre::RaySceneQueryResult &result = query->execute();
		Ogre::RaySceneQueryResult::iterator itr = result.begin();

		if(itr != result.end() && ((to-from).squaredLength() > itr->distance*itr->distance) )
		{
			GAME::PlayState::getSingletonPtr()->getSceneManager()->destroyQuery(query);
			return false;
		}
		else
		{
			GAME::PlayState::getSingletonPtr()->getSceneManager()->destroyQuery(query);
			return true;
		}
	}

	void UnitController::setPath(std::deque<Ogre::Vector3> path)
	{
		moveList.clear();

		for(unsigned int i = 0; i<path.size(); i++)
		{
			path.at(i).y = getPosition().y;
			
			moveList.push_back(path.at(i));
		}
		
		/*
		// SMOOTH PATH
		int i;
		unsigned int sizeBefore;
		sizeBefore = moveList.size();
		// Will smooth the path until it can be smoothed no more.
		do
		{			
			i = moveList.size()-1;
			while(i >= 2)
			{
				if(Walkable(moveList.at(i),moveList.at(i-2)))
				{
					moveList.erase(moveList.begin()+i-1);
				}
				i--;
			}
		} while(sizeBefore > moveList.size());
		*/

		if(!moveList.size())
		{
			moveList.push_back(this->getPosition());
		}

		Direction = moveList.at(0) - this->getPosition();
		Distance = Direction.normalise();
		rotateToDirection(Direction);
	}

	void UnitController::setOffPath(bool set)
	{
		offPath = set;
	}

	Ogre::Vector3 UnitController::getPosition(void)
	{
		return sceneNode->getPosition();
	}

	/*Ogre::Vector3 UnitController::getDirection(void)
	{
		Ogre::Vector3 direction;
		if(unitState == PATH_STATE)
		{
			direction = moveList.at(0)-getPosition();
			direction.normalise();
			return direction;
		}
		else if(unitState == CHASE_STATE)
			if(nearestEnemy)
			{
				direction = nearestEnemy->getPosition()-getPosition();
				direction.normalise();
				return direction;
			}
			else
				return Ogre::Vector3::ZERO;
		else
			return Ogre::Vector3::ZERO;
	}*/

	void UnitController::enemyUnitInRange(UnitController *enemy)
	{
		if( enemy && Walkable(enemy->sceneNode->getPosition(), getPosition()) )
		{
			if( nearestEnemy )
			{
				float distanceToCurrent = nearestEnemy->sceneNode->getPosition().squaredDistance(sceneNode->getPosition());
				float distanceToOther = enemy->sceneNode->getPosition().squaredDistance(sceneNode->getPosition());
				// if new enemy is closer to old nearestEnemy.
				if(distanceToCurrent > distanceToOther)
				{
					nearestEnemy = enemy;
				}
			}
			// No nearest Enemy, make new one.
			else
			{
				nearestEnemy = enemy;
			}
		}		
	}

	void UnitController::inCombat(UnitController *enemy)
	{
		if(combatEnemy)
		{
			float distanceToCurrent = combatEnemy->sceneNode->getPosition().squaredDistance(sceneNode->getPosition());
			float distanceToOther = enemy->sceneNode->getPosition().squaredDistance(sceneNode->getPosition());
			// if new enemy is closer to old combatEnemy.
			if(distanceToCurrent > distanceToOther)
			{
				combatEnemy = enemy;
			}
		}
		// No nearest Enemy, make new one.
		else
		{
			combatEnemy = enemy;
		}
	}

	void UnitController::cleanUpEnemyPointers(void)
	{
		nearestEnemy = NULL;
		combatEnemy = NULL;
	}

	UnitState UnitController::getUnitState(void)
	{
		return unitState;
	}

	void UnitController::damage(int amount)
	{
		if(rand()%100 > evasion) // evasion% chance to not be damaged.
		{
			health -= std::max(amount - defense, 1);
			if(health < 0)
				health = 0;
		}
	}

	void UnitController::kill(void)
	{
		// Ensures that the selected unit is set to null if it is the
		//  same as the unit being killed
		GAME::PlayState::getSingletonPtr()->checkUnitPointerConflicts(this);

		sceneNode->detachAllObjects();
		sceneNode->removeAndDestroyAllChildren();

		//TheApplication.getGame()->getSceneManager()->destroySceneNode(sceneNode);

		GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->destroyManualObject(combatCircle);
		GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->destroyManualObject(detectionCircle);
		
		GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->destroyEntity(entity);

		GAME::PlayState::getSingletonPtr()->getMinimap()->removeUnitOverlay(this);
	}

	int UnitController::getHealth(void){ return health;	}
	int UnitController::getMaxHealth(void){ return maxHealth; }
	int UnitController::getAttack(void){ return attack; }
	void UnitController::setAttack(int a) { attack = a; }
	int UnitController::getDefense(void){ return defense; }
	void UnitController::setDefense(int a) { defense = a; }
	float UnitController::getEvasion(void){ return evasion; }
	float UnitController::getAccuracy(void){ return accuracy; }
	float UnitController::getAttackSpeed(void){ return attackSpeed; }
	float UnitController::getMoveSpeed(void){ return moveSpeed; }
	void UnitController::setMoveSpeed(float speed) { moveSpeed = speed; }

	int UnitController::getCommanderNumber(void) { return commanderNumber; }
	void UnitController::setCommanderNumber(int number) { commanderNumber = number; }

	int UnitController::getConfusedBy(void) { return confusedBy; }
	void UnitController::setConfusedBy(int set) { confusedBy = set; }
	bool UnitController::isHidden(void) { return hidden; }
	void UnitController::setHidden(bool set) { hidden = set; }

	void UnitController::rotateToDirection(Ogre::Vector3 direction)
	{
		Ogre::Vector3 src = sceneNode->getOrientation() * Ogre::Vector3::UNIT_X;
		src.y = 0;
		direction.y = 0;
		src.normalise();
		if ((1.0f + src.dotProduct(direction)) < 0.0001f)
		{
			sceneNode->yaw(Ogre::Degree(180.0f));
		}
		else
		{
			Ogre::Quaternion quat = src.getRotationTo(direction);
			sceneNode->rotate(quat);
		}
	}
}