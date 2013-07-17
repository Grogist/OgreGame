#include "GLgame.h"

namespace GAME
{
	void Game::getQuadTreeInformation(std::vector<UnitNodes> *units, std::vector<LocationNodes> *locations,
		std::vector<FirewallNodes> *firewalls)//, std::vector<ProjectileNodes> *projectiles)
	{
		// Units
		unsigned int i = 0;
		GAME::UnitNodes newNode;

		while(i<commanderList.size())
		{
			unsigned int j = 0;
			while(j<commanderList.at(i)->unitList.size())
			{
				newNode.radii = commanderList.at(i)->unitList.at(j)->detectionRadii;
				newNode.controller = i+1;
				newNode.position = commanderList.at(i)->unitList.at(j)->sceneNode->getPosition();
				newNode.unitController = commanderList.at(i)->unitList.at(j);//&Commander1.unitList.at(i);
				units->push_back(newNode);
				j++;
			}
			i++;
		}

		i = 0;

		// Locations
		GAME::LocationNodes newLocation;
		while(i<resourceList.size())
		{
			newLocation.dimensions = resourceList.at(i).mDimension;
			newLocation.position = resourceList.at(i).mPosition;
			newLocation.location = &resourceList.at(i);
			locations->push_back(newLocation);
			i++;
		}
		i = 0;
		while(i<objectiveList.size())
		{
			newLocation.dimensions = objectiveList.at(i).mDimension;
			newLocation.position = objectiveList.at(i).mPosition;
			newLocation.location = &objectiveList.at(i);
			locations->push_back(newLocation);
			i++;
		}
		i = 0;
		while(i<baseList.size())
		{
			newLocation.dimensions = baseList.at(i).mDimension;
			newLocation.position = baseList.at(i).mPosition;
			newLocation.location = &baseList.at(i);
			newLocation.isBase = true;
			locations->push_back(newLocation);
			i++;
		}

		// Firewalls
		i = 0;
		GAME::FirewallNodes newFirewall;
		while(i<firewallList.size())
		{
			newFirewall.dimensions = firewallList.at(i).mDimension;
			newFirewall.position = firewallList.at(i).getPosition();
			newFirewall.firewall = &firewallList.at(i);
			firewalls->push_back(newFirewall);
			i++;
		}
	}

	// units can be changed to a pointer that has UnitNodes removed when no longer needed.
	//  This will use less memory, but is more complicated.
	void Game::checkUnitUnit(QuadtreeNode *node, std::vector<UnitNodes> units)
	{
		units.insert(units.begin(), node->units.begin(), node->units.end());
		if(node->children)
		{
			for(int i=0; i<4; i++)
			{
				checkUnitUnit(&node->children[i], units);
			}
		}
		// If a QuadtreeNode has no children.
		//   This is a LEAF. All nodes in std::vector units are in a possible collision.
		else
		{
			// Check each unit in units with every other unit in units.
			for(unsigned int i = 0; i<units.size(); i++)
			{
				GAME::UnitNodes *unitsI = &units.at(i);
				// Must be called to clean up the unitController enemy pointers first.
				unitsI->unitController->cleanUpEnemyPointers();
				Ogre::Vector3 ILocation = unitsI->unitController->getPosition();
				ILocation.y = 0;

				float unitsISquaredCombatRadii = float(unitsI->unitController->combatRadii
					* unitsI->unitController->combatRadii);
				float unitsISquaredDetectionRadii = float(unitsI->unitController->detectionRadii
					* unitsI->unitController->detectionRadii);

				for(unsigned int j = 0; j<units.size(); j++)
				{
					GAME::UnitNodes *unitsJ = &units.at(j);

					if( i==j || unitsJ->unitController->isHidden() )
						continue;

					if( !unitsI->unitController->getConfusedBy() &&	unitsI->controller != unitsJ->controller &&
						unitsJ->unitController->getConfusedBy() != unitsI->controller )
					{
						// Improves preformance by doing this less often.
						Ogre::Vector3 JLocation = unitsJ->unitController->getPosition();
						JLocation.y = 0;
						float squaredDistance = ILocation.squaredDistance(JLocation);

						// CHECK FOR COMBAT
						if (unitsISquaredCombatRadii >= squaredDistance)
						{
							//units.at(i) IN RANGE OF units.at(j).
							unitsI->unitController->inCombat(unitsJ->unitController);
						}
						// CHECK FOR DETECTION
						if (unitsISquaredDetectionRadii >= squaredDistance)
						{
							//units.at(i) IN RANGE OF units.at(j).
							unitsI->unitController->enemyUnitInRange(unitsJ->unitController);
						}
					}
					else if( unitsI->unitController->getConfusedBy() &&
						unitsI->unitController->getConfusedBy() != unitsJ->controller )
					{
						Ogre::Vector3 JLocation = unitsJ->unitController->getPosition();
						JLocation.y = 0;

						float squaredDistance = ILocation.squaredDistance(JLocation);


						// CHECK FOR COMBAT
						if (unitsISquaredCombatRadii >= squaredDistance)
						{
							//units.at(i) IN RANGE OF units.at(j).
							unitsI->unitController->inCombat(unitsJ->unitController);
						}
						// CHECK FOR DETECTION
						if (unitsISquaredDetectionRadii >= squaredDistance)
						{
							//units.at(i) IN RANGE OF units.at(j).
							unitsI->unitController->enemyUnitInRange(unitsJ->unitController);
						}
					}
				}
			}
		}
		// Should not need since the data is passed, not the memory location.
		//units.erase(units.end() - node->units.size(), units.end());
	}

	inline bool isInLocation(Ogre::Vector3 unit, Ogre::Vector3 location, Ogre::Vector2 dimension)
	{
		if( abs(unit.x-location.x) < dimension.x/2 && abs(unit.z - location.z) < dimension.y/2 )
			return true;
		else
			return false;
	}

	void Game::checkUnitLocation(QuadtreeNode *node, std::vector<UnitNodes> units, std::vector<LocationNodes> locations)
	{
		units.insert(units.begin(), node->units.begin(), node->units.end());
		locations.insert(locations.begin(), node->locations.begin(), node->locations.end());
		if(node->children)
		{
			for(int i=0; i<4; i++)
			{
				checkUnitLocation(&node->children[i], units, locations);
			}
		}
		else
		{
			for(unsigned int i = 0; i<locations.size(); i++)
			{
				if(locations.at(i).isBase)
				{
					//MAKE BETTER
					if(locations.at(i).location->controlledBy != locations.at(i).location->oldControlledBy)
					{
						locations.at(i).location->oldControlledBy = locations.at(i).location->controlledBy;
						setBorderMaterial(locations.at(i).location);
					}

					continue;
				}
				bool locationTaken = false;
				bool locationAlreadyTakenInFrame = false;
				int locationTakenBy = 0;
				GAME::LocationNodes *l = &locations.at(i);
				Ogre::Vector3 locationPosition = l->location->getPosition();
				Ogre::Vector2 locationDimensions = l->dimensions;
				//Ogre::Vector2 halflocationDimensions = locations.at(i).dimensions/2;
				for(unsigned int j = 0; j<units.size(); j++)
				{
					GAME::UnitNodes *u = &units.at(j);
					Ogre::Vector3 unitPosition = u->unitController->getPosition();					
					if(isInLocation(unitPosition, locationPosition, locationDimensions))
					{
						if(locationAlreadyTakenInFrame == false)
						{
							locationTaken = true;
							locationAlreadyTakenInFrame = true;
							if(u->unitController->getConfusedBy())
								locationTakenBy = u->unitController->getConfusedBy();
							else
								locationTakenBy = u->controller;
						}
						else if(locationAlreadyTakenInFrame == true)
						{
							if( u->unitController->getConfusedBy() )
							{
								if(locationTakenBy != u->unitController->getConfusedBy())
									locationTaken = false;
							}
							else if( locationTakenBy != u->controller )
								locationTaken = false;
						}
					}
				}

				if(locationTaken && locationTakenBy)
					l->location->controlledBy = locationTakenBy;
				else
					l->location->controlledBy = 0;
				
				if(l->location->controlledBy != l->location->oldControlledBy)
				{
					l->location->oldControlledBy = l->location->controlledBy;
					setBorderMaterial(l->location);
				}
			}
		}
		// Should not need since the data is passed, not the memory location.
		//units.erase(units.end() - node->units.size(), units.end());
		//locations.erase(units.end() - node->locations.size(), units.end());
	}

	void Game::checkUnitFirewall(QuadtreeNode *node, std::vector<UnitNodes> units, std::vector<FirewallNodes> firewalls)
	{
		units.insert(units.begin(), node->units.begin(), node->units.end());
		firewalls.insert(firewalls.begin(), node->firewalls.begin(), node->firewalls.end());
		if(node->children)
		{
			for(int i=0; i<4; i++)
			{
				checkUnitFirewall(&node->children[i], units, firewalls);
			}
		}
		else
		{
			for(unsigned int i = 0; i<firewalls.size(); i++)
			{
				GAME::FirewallNodes *f = &firewalls.at(i);
				Ogre::Vector3 locationPosition = f->firewall->getPosition();
				Ogre::Vector2 locationDimensions = f->dimensions;				
				for(unsigned int j = 0; j<units.size(); j++)
				{
					Ogre::Vector3 unitPosition = units.at(j).unitController->getPosition();

					if(isInLocation(unitPosition, locationPosition, locationDimensions))
					{
						if(f->firewall->damageTimer.getMilliseconds() > f->firewall->damageRate)
						{
							units.at(j).unitController->damage(f->firewall->damage);
							f->firewall->damageTimer.reset();
						}
					}
				}
			}
		}
		// Should not need since the data is passed, not the memory location.
		//units.erase(units.end() - node->units.size(), units.end());
		//firewalls.erase(units.end() - node->locations.size(), units.end());
	}

	void Game::checkCollision(void)
	{
		// Go down each branch of tree, storing units in a list.
		// At the end of each branch, check each unit in the list
		//   against each other.
		// FOR DETECTION:
			// If not collision, continue,
			// If collision unit.SetDirection(*otherUnit.Position)
		// FOR COLLISION:
			// Melee units start attacking

		std::vector<UnitNodes> unitNodes;
		std::vector<LocationNodes> locationNodes;
		std::vector<FirewallNodes> firewallNodes;
		//std::vector<ProjectileNodes> projectileNodes;
		checkUnitUnit(&detectionTree.root, unitNodes);
		checkUnitLocation(&detectionTree.root, unitNodes, locationNodes);
		// Only need to check if a firewall exists.
		if(firewallList.size() > 0)
			checkUnitFirewall(&detectionTree.root, unitNodes, firewallNodes);
		collisionTimer.reset();		
	}
}