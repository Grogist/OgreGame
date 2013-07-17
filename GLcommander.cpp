#include "GLcommander.h"

#include "GLgame.h"
#include "GLapplication.h"
#include "GLPlayState.h"

namespace GAME
{
	static const int FastUnit_Cost = 60;
	static const int NormalUnit_Cost = 80;
	static const int SlowUnit_Cost = 100;
	static const int RangedUnit_Cost = 125;

	Commander::Commander() : commanderScore(0), commanderResources(280), baseHealth(1000), nextUnit(UnitType_Normal),
		nextUnitCost(NormalUnit_Cost)
	{
		genTimer.reset();
	}

	Commander::~Commander()
	{
		while(!unitList.empty())
		{
			delete unitList.back();
			unitList.back() = NULL;
			unitList.pop_back();
		}
	}

	void Commander::runAI(Ogre::Real time)
	{
		if (genTimer.getMilliseconds()>1000)
		{
			// 1. Pick a location with highest priority that Commander currently does not control,
			//    or have a unit currently headed toward (unit.targetLocation).
			// 1a. Create an available list of locations, initially consisting of all locations.
				// DONE.
			// 1b. Remove locations from the available list for each unitList.at(i).targetLocation. Will
			//     be left with nothing but available locations.
				// WILL BE DONE IN STEP 2.
			// 1c. From available locations find the location with the highest priority.
				// highest is the location in availableList that has the highest priority. ie. the chosen location.
				unsigned int pickedLocation = -1;
				int highestPriority = 0;
				int currentPriority;
				if(!availableList.empty())
				{
					for(unsigned int i = 0; i < availableList.size(); i++)
					{
						currentPriority = availableList.at(i).second;
						if(highestPriority < currentPriority)
						{
							pickedLocation = i;
							highestPriority = currentPriority;
						}
						else if(highestPriority == currentPriority)
						{
							// 50% chance of changing currentHighest.
							int random = std::rand()%2;
							if(random == 0)
								pickedLocation = i;
								highestPriority = currentPriority;
						}
					}
				}
				// Reinforce a random location.
				else
				{
					int randType = rand() % 2;
					int randLocation;
					switch(randType)
					{
					case 0:
						{
							randLocation = rand() % resourceList.size();
							availableList.push_back(resourceList.at(randLocation));
							pickedLocation = 0;
							break;
						}
					case 1:
						{
							randLocation = rand() % objectiveList.size();
							availableList.push_back(resourceList.at(randLocation));
							pickedLocation = 0;
							break;
						}
					}
				}

			// 2. Pick a unit that is being the most useless, send that unit to chosen location.
			// 2a. Find the unit that has the lowest priority targetLocation, or a unit with no targetLocation.			
				// currentLowest is the location in unitList that has the lowest priority. ie. the chosen unit.
				unsigned int pickedUnit = -1;
				int lowestPriority = 0;
				//int currentPriority;
				for(unsigned int i = 0; i < unitList.size(); i++)
				{
					if(!unitList.at(i)->targetLocation.first)
					{
						pickedUnit = i;
						break;
					}
					
					currentPriority = unitList.at(i)->targetLocation.second;
					if(lowestPriority > currentPriority)
					{
						pickedUnit = i;
						lowestPriority = currentPriority;
					}
					else if(lowestPriority == currentPriority)
					{
						// 50% chance of changing currentHighest.
						int random = std::rand()%2;
						if(random == 0)
							pickedUnit = i;
							lowestPriority = currentPriority;
					}
				}

			// 2b. unitList.at(i).setPath(path.constructPath(unitList.at(i).getPosition(), destination));
				if(!unitList.empty() && pickedUnit >= 0 && pickedUnit < unitList.size() && pickedLocation >= 0 && pickedLocation < availableList.size())
				{
					unitList.at(pickedUnit)->setPath( path.constructPath( unitList.at(pickedUnit)->getPosition(),
						availableList.at(pickedLocation).first->getPosition() ) );

					if(unitList.at(pickedUnit)->targetLocation.first)
					{
						bool isTargeted = false;
						for(unsigned int size = 0; size < unitList.size(); size++)
						{
							if(size != pickedUnit && unitList.at(size)->targetLocation == availableList.at(pickedLocation))
							{
								isTargeted = true;
								break;
							}
						}

						if(!isTargeted)
							availableList.push_back(unitList.at(pickedUnit)->targetLocation);
					}

					unitList.at(pickedUnit)->targetLocation = availableList.at(pickedLocation);

					// availableList.at(pickedLocation) is not longer available, remove it.
					if(!availableList.empty())
						availableList.erase(availableList.begin()+pickedLocation);
				}

			genTimer.reset();
		}

		// Update each unit in unitList.
		unsigned int i=0;
		while(i<unitList.size())
		{
			unitList.at(i)->update(time);
			if(unitList.at(i)->getHealth() <= 0)
			{
				// MAKE BETTER!
				if(unitList.at(i)->targetLocation.first)
				{
					bool isInAvailableList = false;
					for(unsigned int size = 0; size < availableList.size(); size++)
					{
						if(unitList.at(i)->targetLocation == availableList.at(size))
						{
							isInAvailableList = true;
							break;
						}
					}

					if(!isInAvailableList)
						availableList.push_back(unitList.at(i)->targetLocation);
				}

				unitList.at(i)->kill();
				delete unitList.at(i);
				unitList.at(i) = NULL;
				unitList.erase(unitList.begin()+i);
				mTimer.StopTimer(i);
				mTimer.DecrementTimers(i);

				// Flushes junk data caused by removing a unit from unitList.
				Ogre::Vector2 worldsize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();
				GAME::PlayState::getSingletonPtr()->getGame()->detectionTree.Rebuild(worldsize.x,-worldsize.x/2,-worldsize.y/2);
				GAME::PlayState::getSingletonPtr()->getGame()->rebuildQuadtreeTimer.reset();
				GAME::PlayState::getSingletonPtr()->getGame()->checkCollision();
				//TheApplication.getGame()->checkCollisionTimer.reset();
				i--;
				continue;
			}
			++i;
		}
		
		// Check for condition to spawn a unit.
		// Differentiate between Player's Commander and other. Nonplayer Commanders spawn units at a greater rate.

		// The Commander will pick the next unit (based on something and wait until it has the resources to make it.
		if(commanderNumber == 1 && commanderResources > nextUnitCost*2 )
		{
			commanderResources -= nextUnitCost*2;

			createUnit();			
		}
		else if(commanderNumber != 1 && commanderResources > nextUnitCost)
		{
			commanderResources -= nextUnitCost;

			createUnit();
		}
	}

	void Commander::addUnit(GAME::UnitController *newUnit)
	{
		unitList.push_back(newUnit);
	}

	void createCircle(Ogre::ManualObject *circle, Ogre::ColourValue colour, Ogre::Real radius)
	{
		float const accuracy = 20.0f, thickness = 2.0f; 

		circle->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		unsigned point_index = 0;

		for(float theta = 0; theta <= 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy)
		{
			circle->position(radius * cos(theta),
				0, radius * sin(theta));
			circle->colour(colour);
			circle->position(radius * cos(theta-Ogre::Math::PI/accuracy),
				0, radius * sin(theta-Ogre::Math::PI/accuracy));
			circle->colour(colour);
			circle->position((radius-thickness) * cos(theta-Ogre::Math::PI/accuracy),
				0, (radius-thickness) * sin(theta-Ogre::Math::PI/accuracy));
			circle->colour(colour);
			circle->position((radius-thickness) * cos(theta),
				0, (radius-thickness) * sin(theta));
			circle->colour(colour);
			circle->quad(point_index,point_index+1,point_index+2,point_index+3);
			point_index+=4;
		}
		circle->end();
	}

	void Commander::createUnit()
	{
			createUnit(homeBase->getPosition(), nextUnit);

			// Choose next unit.
			int rand = std::rand()%4;
			switch (rand)
			{
			case 0:
				nextUnit = UnitType_Slow;
				nextUnitCost = FastUnit_Cost;
				break;
			case 1:
				nextUnit = UnitType_Normal;
				nextUnitCost = NormalUnit_Cost;
				break;
			case 2:
				nextUnit = UnitType_Fast;
				nextUnitCost = SlowUnit_Cost;
				break;
			case 3:
				nextUnit = UnitType_Ranged;
				nextUnitCost = RangedUnit_Cost;
				break;
			}
	}

	void Commander::createUnit(Ogre::Vector3 position, UnitType type)
	{
		char *mesh;
		UnitController *newUnit;

		switch(type)
		{
		case UnitType_Fast:
			newUnit = new FastUnitController;
			mesh = "Pyramid.mesh";
			break;
		case UnitType_Normal:
			newUnit = new NormalUnitController;
			mesh = "Cube.mesh";
			break;
		case UnitType_Slow:
			newUnit = new SlowUnitController;
			mesh = "Dodecahedron.mesh";
			break;
		case UnitType_Ranged:
			newUnit = new RangedUnitController;
			mesh = "Octahedron.mesh";
			break;
		default:
			newUnit = new NormalUnitController;
			mesh = "Cube.mesh";
		}

		char numstr[21];
		char intermediate[21];
		sprintf(intermediate,"%d",GAME::PlayState::getSingletonPtr()->getGame()->numberOfEntities);
		sprintf(numstr,"Unit");
		strcat(numstr, intermediate);
		newUnit->entity = GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->createEntity(numstr, mesh);
		newUnit->entity->setQueryFlags(SELECTABLE_MASK | UNIT_MASK);

		newUnit->sceneNode = GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->getRootSceneNode()->createChildSceneNode(
			std::string(numstr), position);
		newUnit->sceneNode->attachObject(newUnit->entity);
		
		if(newUnit->entity->getMesh()->getName() == Ogre::String("Cube.mesh"))
		{
			newUnit->sceneNode->setScale(15.0f,15.0f,15.0f);
			if(colour == BLUE)
				newUnit->entity->setMaterialName("Textures/CubeBlue");
			else if(colour == RED)
				newUnit->entity->setMaterialName("Textures/CubeRed");
			else if(colour == GREEN)
				newUnit->entity->setMaterialName("Textures/CubeGreen");
			else if(colour == DARKBLUE)
				newUnit->entity->setMaterialName("Textures/CubeDarkBlue");
		}
		else if(newUnit->entity->getMesh()->getName() == Ogre::String("Pyramid.mesh"))
		{
			newUnit->sceneNode->setScale(15.0f,15.0f,15.0f);
			if(colour == BLUE)
				newUnit->entity->setMaterialName("Textures/PyramidBlue");
			else if(colour == RED)
				newUnit->entity->setMaterialName("Textures/PyramidRed");
			if(colour == GREEN)
				newUnit->entity->setMaterialName("Textures/PyramidGreen");
			else if(colour == DARKBLUE)
				newUnit->entity->setMaterialName("Textures/PyramidDarkBlue");
		}
		else if(newUnit->entity->getMesh()->getName() == Ogre::String("Octahedron.mesh"))
		{
			newUnit->sceneNode->setScale(15.0f,15.0f,15.0f);
			if(colour == BLUE)
				newUnit->entity->setMaterialName("Textures/OctahedronBlue");
			else if(colour == RED)
				newUnit->entity->setMaterialName("Textures/OctahedronRed");
			else if(colour == GREEN)
				newUnit->entity->setMaterialName("Textures/OctahedronGreen");
			else if(colour == DARKBLUE)
				newUnit->entity->setMaterialName("Textures/OctahedronDarkBlue");
		}
		else if(newUnit->entity->getMesh()->getName() == Ogre::String("Dodecahedron.mesh"))
		{
			newUnit->sceneNode->setScale(30.0f,30.0f,30.0f);
			if(colour == BLUE)
				newUnit->entity->setMaterialName("Textures/DodecahedronBlue");
			else if(colour == RED)
				newUnit->entity->setMaterialName("Textures/DodecahedronRed");
			else if(colour == GREEN)
				newUnit->entity->setMaterialName("Textures/DodecahedronGreen");
			else if(colour == DARKBLUE)
				newUnit->entity->setMaterialName("Textures/DodecahedronDarkBlue");
		}

		newUnit->entity->setCastShadows(false);

		newUnit->initialize();

		newUnit->combatCircleNode = newUnit->sceneNode->createChildSceneNode();
		newUnit->combatCircleNode->setInheritScale(false);
		sprintf(intermediate,"%d",GAME::PlayState::getSingletonPtr()->getGame()->numberOfEntities);
		sprintf(numstr,"CombatCircle");
		strcat(numstr, intermediate);
		newUnit->combatCircle = GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->createManualObject(numstr);
		createCircle(newUnit->combatCircle, Ogre::ColourValue(1.0f,0.0,0.0), float(newUnit->combatRadii));
		newUnit->combatCircle->setQueryFlags(CIRCLE_MASK);
		newUnit->combatCircleNode->attachObject(newUnit->combatCircle);
		newUnit->combatCircleNode->setVisible(false);

		newUnit->detectionCircleNode = newUnit->sceneNode->createChildSceneNode();
		newUnit->detectionCircleNode->setInheritScale(false);
		sprintf(intermediate,"%d",GAME::PlayState::getSingletonPtr()->getGame()->numberOfEntities);
		sprintf(numstr,"DetectionCircle");
		strcat(numstr, intermediate);
		newUnit->detectionCircle = GAME::PlayState::getSingletonPtr()->getGame()->getSceneManager()->createManualObject(numstr);
		createCircle(newUnit->detectionCircle, Ogre::ColourValue(0.0f,1.0,1.0), float(newUnit->detectionRadii));
		newUnit->detectionCircle->setQueryFlags(CIRCLE_MASK);
		newUnit->detectionCircleNode->attachObject(newUnit->detectionCircle);
		newUnit->detectionCircleNode->setVisible(false);

		newUnit->setCommanderNumber(commanderNumber);

		GAME::PlayState::getSingletonPtr()->getGame()->numberOfEntities++;
		unitList.push_back(newUnit);

		//MAKE OVERLAY FOR UNIT.
		GAME::PlayState::getSingletonPtr()->getMinimap()->makeUnitOverlay(unitList.back());

		// Flushes junk data caused by adding a unit from unitList.
		Ogre::Vector2 worldsize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();
		GAME::PlayState::getSingletonPtr()->getGame()->detectionTree.Rebuild(worldsize.x,-worldsize.x/2,-worldsize.y/2);
		GAME::PlayState::getSingletonPtr()->getGame()->rebuildQuadtreeTimer.reset();
	}


	void Commander::setPointers(std::deque<GAME::Location> *resource,
			std::deque<GAME::Location> *objective, std::deque<GAME::Location> *base,
			std::deque<Ogre::SceneNode *> *wall)
	{
		wallList = wall;

		unsigned int i;
		float maxDistance = 0;
		// The map should never be big enough to reach initial minDistance.
		float minDistance = 9999999999.0f;
		float squaredDistance;
		// Value can add/subtract up to 3 from initial priority.
		int value;
		float averageValue = 0;
		int maxValue = 0;
		// No location should ever be worth than initial minValue.
		int minValue = 9999;

		for(i=0; i<resource->size(); i++)
		{
			squaredDistance = resource->at(i).getPosition().squaredDistance(homeBase->getPosition());
			if(maxDistance < squaredDistance)
				maxDistance = squaredDistance;
			else if(minDistance > squaredDistance)
				minDistance = squaredDistance;
			value = resource->at(i).locationValue;
			averageValue += value;			
			if(maxValue < value)
				minValue = value;
			else if(minDistance > value)
				minValue = value;
		}

		for(i=0; i<objective->size(); i++)
		{
			squaredDistance = objective->at(i).getPosition().squaredDistance(homeBase->getPosition());
			if(maxDistance < squaredDistance)
				maxDistance = squaredDistance;
			else if(minDistance > squaredDistance)
				minDistance = squaredDistance;
			value = objective->at(i).locationValue;
			averageValue += value;		
			if(maxValue < value)
				minValue = value;
			else if(minDistance > value)
				minValue = value;
		}
		/*********************
		if(GAMETYPE == ANNIHILATION)
		{
			for(i=0; i<base->size(); i++)
			{
				squaredDistance = base->at(i).getPosition().squaredDistance(homeBase->getPosition());
				if(maxDistance < squaredDistance)
					maxDistance = squaredDistance;
				else if(minDistance > squaredDistance)
					minDistance = squaredDistance;
				value = base->at(i).locationValue;
				averageValue += value;		
				if(maxValue < value)
					minValue = value;
				else if(minDistance > value)
					minValue = value;
			}
		}
		*********************/

		// minDistance and maxDistance are found. (They are the squared distance)
		// Set priority based on distance.
		float segment = (maxDistance - minDistance) / DEFAULT_MaxPriority;
		averageValue = averageValue / (resource->size() + objective->size());

		std::pair<GAME::Location *, int> tmp;
		
		for(i=0; i<resource->size(); i++)
		{
			tmp.first = &resource->at(i);
			squaredDistance = tmp.first->getPosition().squaredDistance(homeBase->getPosition());
			// Add to priority.
			if(tmp.first->locationValue > averageValue)
			{
				value = int( (tmp.first->locationValue - averageValue) / (maxValue - averageValue) );
				for(int j=DEFAULT_ValuePriorityAdjust; j=0; j--)
				{
					if(value >= j/DEFAULT_ValuePriorityAdjust)
					{
						value = j;
						break;
					}
				}
			}
			// Subtract from priority.
			else if(tmp.first->locationValue < averageValue)
			{
				value = int( (averageValue - tmp.first->locationValue) / (averageValue - minValue) );
				for(int j=0; j=DEFAULT_ValuePriorityAdjust; j++)
				{
					if(value <= j/DEFAULT_ValuePriorityAdjust)
					{
						value = -j;
						break;
					}
				}
			}
			tmp.second = std::min(10, std::max(1, DEFAULT_MaxPriority - int((squaredDistance - minDistance) / segment) + value));
			resourceList.push_back(tmp);
			availableList.push_back(tmp);
		}
		for(i=0; i<objective->size(); i++)
		{
			tmp.first = &objective->at(i);
			squaredDistance = tmp.first->getPosition().squaredDistance(homeBase->getPosition());
			// Add to priority.
			if(tmp.first->locationValue > averageValue)
			{
				value = int( (tmp.first->locationValue - averageValue) / (maxValue - averageValue) );
				for(int j=DEFAULT_ValuePriorityAdjust; j=0; j--)
				{
					if(value >= j/DEFAULT_ValuePriorityAdjust)
					{
						value = j;
						break;
					}
				}				
			}				
			// Subtract from priority.
			else if(tmp.first->locationValue < averageValue)
			{
				value = int( (averageValue - tmp.first->locationValue) / (averageValue - minValue) );
				for(int j=0; j=DEFAULT_ValuePriorityAdjust; j++)
				{
					if(value <= j/DEFAULT_ValuePriorityAdjust)
					{
						value = -j;
						break;
					}
				}
			}
			tmp.second = std::min(10, std::max(1, DEFAULT_MaxPriority - int((squaredDistance - minDistance) / segment) + value));
			objectiveList.push_back(tmp);
			availableList.push_back(tmp);
		}
		for(i=0; i<base->size(); i++)
		{
			tmp.first = &base->at(i);
			tmp.second = 0;
			/**********
			if(GAMETYPE == ANNIHILATION) MODIFIY
			{
				squaredDistance = tmp.first->getPosition().squaredDistance(homeBase->getPosition());
				tmp.second =  std::max(DEFAULT_MaxPriority - int((squaredDistance - minDistance) / segment),1);
			}
			*********/
			baseList.push_back(tmp);
			//availableList.push_back(tmp);
		}

		TheApplication.getRoot()->addFrameListener(&mTimer);
	}

	int Commander::getScore(void)
	{
		return commanderScore;
	}

	void Commander::addToScore(int add)
	{
		commanderScore += add;
	}

	void Commander::setHomeBase(GAME::Location* base, int number)
	{
		homeBase = base;
		commanderNumber = number;
	}

	void Commander::setUnitColour(UnitColour colour)
	{
		this->colour = colour;
	}

	UnitColour Commander::getUnitColour(void)
	{
		return colour;
	}

	int Commander::getResources(void)
	{
		return commanderResources;
	}

	void Commander::addToResources(int add)
	{
		commanderResources += add;
	}

	int Commander::getLocationPriority(int index, LocationType type)
	{
		switch(type)
		{
		case (BASE):
			if(0 <= index && index < (signed)baseList.size())
				return baseList.at(index).second;
			else
				return 0;
			break;
		case (RESOURCE):
			if(0 <= index && index < (signed)resourceList.size())
				return resourceList.at(index).second;
			else
				return 0;
			break;
		case (OBJECTIVE):
			if(0 <= index && index < (signed)objectiveList.size())
				return objectiveList.at(index).second;
			else
				return 0;
			break;
		default:
			return 0;
			break;
		}
	}

	int Commander::getCommanderNumber(void)
	{
		return commanderNumber;
	}

	int Commander::getBaseHealth(void)
	{
		return baseHealth;
	}
	
	void Commander::damageBase(int damage)
	{
		baseHealth -= damage;
	}

	void Commander::modifyLocationPriority(int index, int value, LocationType type)
	{
		switch(type)
		{
		case (BASE):
			baseList.at(index).second = std::max(std::min(10, baseList.at(index).second + value), 0);
			break;
		case(RESOURCE):
			resourceList.at(index).second = std::max(std::min(10, resourceList.at(index).second + value), 0);
			break;
		case(OBJECTIVE):
			objectiveList.at(index).second = std::max(std::min(10, objectiveList.at(index).second + value), 0);
			break;
		}
	}

	void Commander::confuseUnit(GAME::UnitController *unit, int confusedBy)
	{
		bool found = false;
		unsigned int index;
		for(index = 0; index<unitList.size(); ++index)
		{
			if(unitList.at(index) == unit)
			{
				found = true;
				break;
			}
		}
		if(found)
		{
			unitList.at(index)->setConfusedBy(confusedBy);
			mTimer.AddTimer(10000, &Commander::unconfuseUnit, this, index, index, false, true);
		}
	}

	bool Commander::unconfuseUnit(int Timer_ID, int unitIndex)
	{
		unitList.at(unitIndex)->setConfusedBy(0);
		return true;
	}

	void Commander::hideUnit(GAME::UnitController *unit)
	{
		bool found = false;
		unsigned int index;
		for(index = 0; index<unitList.size(); ++index)
		{
			if(unitList.at(index) == unit)
			{
				found = true;
				break;
			}
		}
		if(found)
		{
			unitList.at(index)->setHidden(true);
			mTimer.AddTimer(10000, &Commander::unhideUnit, this, index, index, false, true);
		}
	}

	bool Commander::unhideUnit(int Timer_ID, int unitIndex)
	{
		unitList.at(unitIndex)->setHidden(false);
		return true;
	}

	void Commander::teleportUnit(GAME::UnitController *unit, Ogre::Vector3 destination)
	{
		unit->sceneNode->setPosition(destination);
		if(unit->targetLocation.first)
			unit->setPath( path.constructPath( unit->getPosition(),	unit->targetLocation.first->getPosition() ) );
		//Ogre::Vector3 position = targetedUnit->getPosition();
		//targetedUnit->sceneNode->setPosition(Ogre::Vector3(mRayPosition.x, position.y, mRayPosition.z));
		//targetedUnit->setOffPath(true);
	}

	void Commander::Cleanup(void)
	{
		TheApplication.getRoot()->removeFrameListener(&mTimer);
		while(!unitList.empty())
		{
			delete unitList.back();
			unitList.back() = NULL;
			unitList.pop_back();
		}

		unitList.clear();
		wallList->clear();

		resourceList.clear();
		objectiveList.clear();
		baseList.clear();

		// ONLY HERE TO COPE WITH COMMANDER MEMORY LEAK.
		path.grid.clear();
		path.came_from_map.clear();

		availableList.clear();
		isTargeted.clear();
	}
}