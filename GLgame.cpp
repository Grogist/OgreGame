#include "GLgame.h"

#include "GLapplication.h"
#include "GLPlayState.h"
#include "GLMenuState.h"

namespace GAME
{
	Game::Game()
	{
		updateLocationTimer.reset();
		rebuildQuadtreeTimer.reset();
		collisionTimer.reset();
	}

	Game::~Game() { }

	// Order in which a scene must be built:
	//  1. In any order: Skybox, lighting, ground, grass, locations, walls, anything else not explicitly stated.
	//  2. Build DetectionTree
	//  3. CommanderList. For each Commander 3a. setPointers, 3b. createGrid
	//  4. Units
	bool Game::createScene(std::string filename)
	{
		// CREATE WORLD
		numberOfEntities = 0;

		SceneMgr->setAmbientLight(Ogre::ColourValue::White);
		//SceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
		SceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

		//std::string maplocation("D:/OgreGame/RTS/Map/");
		std::string maplocation("../Map/");
		
		std::string file = maplocation + filename;
		tinyxml2::XMLDocument doc;
		doc.LoadFile(file.c_str());

		//if(filename.size())
		if(!doc.LoadFile(file.c_str()))
			LoadMap(file);
		else
			return false;

		// Build detectionTree
		detectionTree.Rebuild(WorldSize.x,-WorldSize.x/2,-WorldSize.y/2);

		// CREATE COMMANDER LIST
		std::vector<GAME::UnitColour> potentialColours;
		potentialColours.push_back(RED);
		potentialColours.push_back(GREEN);
		potentialColours.push_back(BLUE);
		potentialColours.push_back(DARKBLUE);

		unsigned int i = 0;
		for(i = 0; i<baseList.size() && i<4; i++)
		{
			GAME::Commander *newCommander = new GAME::Commander();
			commanderList.push_back(newCommander);

			// setHomeBase first.
			commanderList.back()->setHomeBase(&baseList.at(i), i+1);
			commanderList.back()->setPointers(&resourceList, &objectiveList, &baseList, &wallList);
			commanderList.back()->path.constructGrid(wallList, (int)WorldSize.x, (int)WorldSize.y, 50);

			if (i==0)
			{
				for(unsigned int j = 0; j<4; j++)
				{
					if(potentialColours.at(j) == GAME::MenuState::getSingletonPtr()->m_PlayerColour)
					{
						potentialColours.erase(potentialColours.begin() + j);
						break;
					}
				}
				commanderList.at(i)->setUnitColour(GAME::MenuState::getSingletonPtr()->m_PlayerColour);
			}
			else
			{
				int rand = std::rand()%potentialColours.size();
				commanderList.at(i)->setUnitColour(potentialColours.at(rand));
				potentialColours.erase(potentialColours.begin() + rand);
			}
		}

		// RELIC
		TheApplication.getRoot()->addFrameListener(&mAbilityTimer);
		TheApplication.getRoot()->addFrameListener(&mProjectileTimer);
		TheApplication.getRoot()->addFrameListener(&mFirewallTimer);

		Ogre::ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);

		SceneMgr->createParticleSystem("Explosion", "Explosion");

		return true;
	}

	void Game::createLocation(Ogre::Vector3 position, Ogre::Vector2 dimension, char* name, LocationType type, int value)
	{
		GAME::Location newLocation;

		char numstr[21];
		char intermediate[21];
		sprintf(intermediate,"%d",numberOfEntities);
		if(type == RESOURCE)
			sprintf(numstr, "LocaResource");
		else if(type == BASE)
			sprintf(numstr, "LocaBase");
		else if(type == OBJECTIVE)
			sprintf(numstr, "LocaObjective");
		else
			sprintf(numstr,"Location");
		strcat(numstr, intermediate);

		newLocation.mPlane = new Ogre::MovablePlane(numstr);
		newLocation.mPlane->normal = Ogre::Vector3::UNIT_Y;

		newLocation.mDimension = dimension;
		newLocation.mPosition = position;

		// IF NO GIVEN VALUE, CALCULATE IT BASED ON DEFAULT FORMULA
		if(value)
			newLocation.locationValue = value;
		else
			newLocation.locationValue = int((dimension.x + dimension.y) * 0.006f); // 0.006 = (1.2/200)
		newLocation.type = type;

		Ogre::MeshManager::getSingleton().createPlane(numstr,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			,*newLocation.mPlane, dimension.x, dimension.y, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_X);
		newLocation.mEntity = SceneMgr->createEntity(numstr, numstr);
		newLocation.mEntity->setQueryFlags(SELECTABLE_MASK);
		
		newLocation.mSceneNode = SceneMgr->getRootSceneNode()->createChildSceneNode(numstr);
		newLocation.mSceneNode->attachObject(newLocation.mEntity);
		newLocation.mSceneNode->setPosition(position);


		sprintf(numstr,"Border%d",numberOfEntities);
		// Create a manual object for 2D
		Ogre::ManualObject *manual = SceneMgr->createManualObject(numstr);
 
		manual->begin("Custom/FirewallGround", Ogre::RenderOperation::OT_TRIANGLE_LIST);

		Ogre::Vector2 borderSize = dimension * 0.06f;
		float posX = position.x + dimension.x * 0.5f;
		float negX = position.x - dimension.x * 0.5f;
		float posZ = position.z + dimension.y * 0.5f;
		float negZ = position.z - dimension.y * 0.5f;

		unsigned int point_index = 0;
		// Right
		manual->position(posX + borderSize.x, 1, posZ + borderSize.y);
		manual->position(posX + borderSize.x, 1, posZ);
		manual->position(negX - borderSize.x, 1, posZ);
		manual->position(negX - borderSize.x, 1, posZ + borderSize.y);
		manual->quad(point_index,point_index+1,point_index+2,point_index+3);
		point_index+=4;
		// Top
		manual->position(posX + borderSize.x, 1, posZ + borderSize.y);
		manual->position(posX + borderSize.x, 1, negZ - borderSize.y);
		manual->position(posX, 1, negZ - borderSize.y);
		manual->position(posX, 1, posZ + borderSize.y);
		manual->quad(point_index,point_index+1,point_index+2,point_index+3);
		point_index+=4;
		// Bottom
		manual->position(negX, 1, posZ + borderSize.y);
		manual->position(negX, 1, negZ - borderSize.y);
		manual->position(negX - borderSize.x, 1, negZ - borderSize.y);
		manual->position(negX - borderSize.x, 1, posZ + borderSize.y);
		manual->quad(point_index,point_index+1,point_index+2,point_index+3);
		point_index+=4;
		// Left
		manual->position(posX + borderSize.x, 1, negZ);
		manual->position(posX + borderSize.x, 1, negZ - borderSize.y);
		manual->position(negX - borderSize.x, 1, negZ - borderSize.y);
		manual->position(negX - borderSize.x, 1, negZ);
		manual->quad(point_index,point_index+1,point_index+2,point_index+3);
		point_index+=4;

		manual->end();		

		newLocation.m_BorderSceneNode = SceneMgr->getRootSceneNode()->createChildSceneNode(numstr);
		newLocation.m_BorderSceneNode->attachObject(manual);
		newLocation.m_BorderManualObject = manual;

		if(type == RESOURCE)
		{

			newLocation.mEntity->setMaterialName("Custom/TransparentGreen");
			resourceList.push_back(newLocation);
		}
		else if(type == OBJECTIVE)
		{
			newLocation.mEntity->setMaterialName("Custom/TransparentBlue");
			objectiveList.push_back(newLocation);
		}
		else if(type == BASE)
		{
			newLocation.mEntity->setMaterialName("Custom/TransparentRed");
			newLocation.locationValue = 8;
			newLocation.controlledBy = baseList.size()+1;
			//newLocation.oldControlledBy = newLocation.controlledBy;
			baseList.push_back(newLocation);
		}
		numberOfEntities++;
	}

	void Game::createWall(Ogre::Vector3 position, Ogre::Vector3 scale, char* name, char* meshname)
	{
		char numstr[21];
		char intermediate[21];
		sprintf(intermediate,"%d",numberOfEntities);
		sprintf(numstr,"Wall");
		strcat(numstr, intermediate);

		Ogre::Entity *wall = SceneMgr->createEntity(numstr, "cube.mesh");
		wall->setQueryFlags(SELECTABLE_MASK | WALL_MASK);
		wall->setMaterialName("Plasma");
		Ogre::SceneNode *houseNode = SceneMgr->getRootSceneNode()->createChildSceneNode(numstr);
		houseNode->attachObject(wall);
		houseNode->scale(scale);
		houseNode->setPosition(position);
		//houseNode->showBoundingBox(true);
		wallList.push_back(houseNode);

		numberOfEntities++;
	}

	void Game::initSceneManager(Ogre::SceneManager *Manager)
	{
		SceneMgr = Manager;
	}

	Ogre::SceneManager *Game::getSceneManager(void)
	{
		return SceneMgr;
	}

	void Game::setBorderMaterial(GAME::Location *location)
	{
		if(location->controlledBy)
		{
			switch(commanderList.at(location->controlledBy-1)->getUnitColour())
			{
			case BLUE:
				location->m_BorderManualObject->setMaterialName(0, "Custom/BorderBlue");
				break;
			case RED:
				location->m_BorderManualObject->setMaterialName(0, "Custom/BorderRed");
				break;
			case GREEN:
				location->m_BorderManualObject->setMaterialName(0, "Custom/BorderGreen");
				break;
			case DARKBLUE:
				location->m_BorderManualObject->setMaterialName(0, "Custom/BorderDarkBlue");
				break;
			default:
				break;
			}
		}
		else
			location->m_BorderManualObject->setMaterialName(0, "Custom/FirewallGround");
	}

	void Game::update(const Ogre::FrameEvent &evt)
	{
		// FOR INSURANCE AND FOR TESTING PURPOSES ONLY.
		_ASSERTE(_CrtCheckMemory());

		if(updateLocationTimer.getMilliseconds() > 1222.0f)
		{
			// Update the Locations (adds to Commander scores).
			unsigned int i;
			for(i=0; i<resourceList.size(); i++)
			{
				if(resourceList.at(i).controlledBy)
				{
					int value = resourceList.at(i).locationValue;
					int commander = resourceList.at(i).controlledBy - 1;
					commanderList.at(commander)->addToResources(value);
				}
			}
			for(i=0; i<objectiveList.size();i++)
			{
				if(objectiveList.at(i).controlledBy)
				{
					int value = objectiveList.at(i).locationValue;
					int commander = objectiveList.at(i).controlledBy - 1;
					commanderList.at(commander)->addToScore(value);
				}
			}
			for(i=0; i<baseList.size(); i++)
			{
				int value = baseList.at(i).locationValue;
				int commander = baseList.at(i).controlledBy - 1;
				commanderList.at(commander)->addToResources(value);
			}
			updateLocationTimer.reset();

			// Check for victory
			for( unsigned int i = 0; i<commanderList.size(); i++)
			{
				if( commanderList.at(i)->getScore() > NumberofPointsforVictory)
				{
					GAME::PlayState::getSingletonPtr()->setGameWonBy(i+1);
					break;
				}
			}
		}

		if(rebuildQuadtreeTimer.getMilliseconds() > 911.0f)
		{
			detectionTree.Rebuild(WorldSize.x,-WorldSize.x/2,-WorldSize.y/2);
			rebuildQuadtreeTimer.reset();
		}

		if(collisionTimer.getMilliseconds() > 47.0f)
		{
			checkCollision();
		}

		unsigned int i = 0;
		while(i<projectileList.size())
		{
			if(projectileList.at(i).isDestroyed())
			{
				destroyProjectile(i, i);
				//projectileList.erase(projectileList.begin() + i);

				// Flushes junk data caused by removing a projectile from projectileList.
				//detectionTree.Rebuild(WorldSize.x,-WorldSize.x/2,-WorldSize.y/2);
				//rebuildQuadtreeTimer.reset();
				continue;
			}

			projectileList.at(i).update(evt.timeSinceLastFrame);
			i++;
		}

		// Update Commanders. (Commanders will update units.)
		i = 0;
		while(i<commanderList.size())
		{
			commanderList.at(i)->runAI(evt.timeSinceLastFrame);
			if(commanderList.at(i)->getBaseHealth() <= 0)
				TheApplication.setShutdown(true);
			i++;
		}
	}

	std::vector<int> Game::getScore(void)
	{
		std::vector<int> scores;
		for(unsigned int i = 0; i<commanderList.size(); i++)
		{
			scores.push_back(commanderList.at(i)->getScore());
		}
		return scores;
	}

	std::vector<int> Game::getResources(void)
	{
		std::vector<int> resources;
		for(unsigned int i = 0; i<commanderList.size(); i++)
		{
			resources.push_back(commanderList.at(i)->getResources());
		}
		return resources;
	}

	std::deque<GAME::Location> *Game::getResourceList(void)
	{
		return &resourceList;
	}

	std::deque<GAME::Location> *Game::getObjectiveList(void)
	{
		return &objectiveList;
	}

	std::deque<GAME::Location> *Game::getBaseList(void)
	{
		return &baseList;
	}

	std::vector<GAME::Commander *> *Game::getCommanderList(void)
	{
		return &commanderList;
	}

	std::deque<GAME::Projectile> *Game::getProjectileList(void)
	{
		return &projectileList;
	}

	void Game::buffUnits(int fromCommander, float amount)
	{
		if(mAbilityTimer.AddTimer(10000, &Game::unbuffUnits, this, fromCommander, 1, false, true))
		{
			// buff
			if(amount <= 1)
			{
				GAME::Commander *commanderPtr = commanderList.at(fromCommander-1);
				for(unsigned int unit = 0; unit < commanderPtr->unitList.size(); unit++)
				{
					commanderPtr->unitList.at(unit)->buff(amount);
				}
			}
		}		
	}

	bool Game::unbuffUnits(int Timer_ID, int fromCommander)
	{
		GAME::Commander *commanderPtr = commanderList.at(fromCommander-1);
		for(unsigned int unit = 0; unit < commanderPtr->unitList.size(); unit++)
		{
			commanderPtr->unitList.at(unit)->unbuff();
		}

		return true;
	}

	void Game::slowUnits(int fromCommander, float amount)
	{
		if(mAbilityTimer.AddTimer(10000, &Game::unslowUnits, this, fromCommander, 1, false, true))
		{
			// slow
			if(amount <= 1)
			{
				GAME::Commander *commanderPtr;
				for(unsigned int commander = 0;
					commander < commanderList.size(); commander++)
				{
					if(commander == fromCommander-1)
						continue;
					commanderPtr = commanderList.at(commander);
					for(unsigned int unit = 0; unit < commanderPtr->unitList.size(); unit++)
					{
						commanderPtr->unitList.at(unit)->slow(amount);
					}
				}
			}
		}
	}

	bool Game::unslowUnits(int Timer_ID, int fromCommander)
	{
		for(unsigned int commander = 0;
			commander < commanderList.size(); commander++)
		{
			if(commander == fromCommander-1)
				continue;
			GAME::Commander *commanderPtr = commanderList.at(commander);
			for(unsigned int unit = 0; unit < commanderPtr->unitList.size(); unit++)
			{
				commanderPtr->unitList.at(unit)->unslow();
			}
		}

		return true;
	}

	void Game::spawnUnit(void)
	{
		if(commanderList.size() >= 1)
		{

			commanderList.at(0)->createUnit();
		}
	}

	void Game::createProjectile(Ogre::Vector3 position, GAME::UnitController *enemy, float speed, int damage, int commanderNumber)
	{
		char numstr[21];
		char intermediate[21];
		sprintf(intermediate,"%d",numberOfEntities);
		sprintf(numstr,"Projectile");
		strcat(numstr, intermediate);

		Ogre::Entity *ent = SceneMgr->createEntity(numstr, "Bullet.mesh");
		Ogre::SceneNode *node = SceneMgr->getRootSceneNode()->createChildSceneNode(numstr, position);
		switch(commanderList.at(commanderNumber-1)->getUnitColour())
		{
		case BLUE:
			ent->setMaterialName("Textures/BulletBlue");
			break;
		case RED:
			ent->setMaterialName("Textures/BulletRed");
			break;
		case GREEN:
			ent->setMaterialName("Textures/BulletGreen");
			break;
		case DARKBLUE:
			ent->setMaterialName("Textures/BulletDarkBlue");
			break;
		default:
			ent->setMaterialName("Textures/BulletBlue");
			break;
		}
		ent->setQueryFlags(PROJECTILE_MASK);
		node->attachObject(ent);
		node->scale(8.0f,8.0f,8.0f);
		node->translate(0.0f,-55.0f,0.0f);
		GAME::Projectile projectile(node, ent, enemy, speed, damage);

		projectileList.push_back(projectile);
		numberOfEntities++;
	
		mProjectileTimer.AddTimer(1250, &Game::destroyProjectile, this, (int)projectileList.size()-1, (int)projectileList.size()-1, false, true);
	}

	bool Game::destroyProjectile(int Timer_ID, int index)
	{
		mProjectileTimer.StopTimer(index);
		mProjectileTimer.DecrementTimers(index);

		projectileList.at(index).destroy();
		projectileList.erase(projectileList.begin() + index);

		return true;
	}

	void Game::createFirewall(Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		char numstr[21];
		char intermediate[21];
		sprintf(intermediate,"%d",numberOfEntities);
		sprintf(numstr, "Firewall");
		strcat(numstr, intermediate);

		Firewall newFirewall;

		newFirewall.mPlane = new Ogre::MovablePlane(numstr);
		newFirewall.mPlane->normal = Ogre::Vector3::UNIT_Y;

		newFirewall.mDimension = Ogre::Vector2(500.0f, 125.0f);;
		newFirewall.mPosition = position;

		Ogre::MeshManager::getSingleton().createPlane(numstr,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			,*newFirewall.mPlane, 500.0f, 125.0f, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_X);
		newFirewall.mEntity = SceneMgr->createEntity(numstr, numstr);
		
		newFirewall.mSceneNode = SceneMgr->getRootSceneNode()->createChildSceneNode(numstr, position, orientation);
		newFirewall.mSceneNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Math::PI/2));
		newFirewall.mSceneNode->attachObject(newFirewall.mEntity);
		//newFirewall.mSceneNode->setPosition(position);

		newFirewall.mEntity->setMaterialName("Custom/FirewallGround");
		firewallList.push_back(newFirewall);
		numberOfEntities++;

		detectionTree.Rebuild(WorldSize.x,-WorldSize.x/2,-WorldSize.y/2);
		rebuildQuadtreeTimer.reset();

		mFirewallTimer.AddTimer(5000, &Game::destroyFirewall, this, (int)firewallList.size()-1, (int)firewallList.size()-1, false, true);
	}

	bool Game::destroyFirewall(int Timer_ID, int index)
	{
		mFirewallTimer.DecrementTimers(index);
	
		GAME::Firewall *f = &firewallList.at(index);

		f->mSceneNode->detachAllObjects();
		f->mSceneNode->removeAndDestroyAllChildren();

		//TheApplication.getGame()->getSceneManager()->destroySceneNode(f->sceneNode);

		//TheApplication.getGame()->getSceneManager()->
		//Ogre::MeshManager::getSingleton().remove(f->mPlane);
		delete f->mPlane;
		f->mPlane = NULL;
		SceneMgr->destroyEntity(f->mEntity);

		firewallList.erase(firewallList.begin() + index);
		detectionTree.Rebuild(WorldSize.x,-WorldSize.x/2,-WorldSize.y/2);
		rebuildQuadtreeTimer.reset();
		return true;
	}

	Ogre::Vector2 Game::getWorldSize(void) { return WorldSize; }

	void Game::Cleanup(void)
	{
		while(!commanderList.empty())
		{
			commanderList.back()->Cleanup();
			// MEMORY LEAK!!! OMGZ!!!
			//delete commanderList.back();
			commanderList.back() = NULL;
			commanderList.pop_back();
		}

		while(!resourceList.empty())
		{
			delete resourceList.back().mPlane;
			resourceList.pop_back();
		}
		while(!objectiveList.empty())
		{
			delete objectiveList.back().mPlane;
			objectiveList.pop_back();
		}
		while(!baseList.empty())
		{
			delete baseList.back().mPlane;
			baseList.pop_back();
		}		

		commanderList.clear();		
		resourceList.clear();
		objectiveList.clear();
		baseList.clear();
		wallList.clear();
		projectileList.clear();
		firewallList.clear();

		SceneMgr->destroyStaticGeometry("GrassArea");
		Ogre::MeshManager::getSingleton().removeAll();
		SceneMgr->destroyEntity("GroundEntity");

		//RELIC
		TheApplication.getRoot()->removeFrameListener(&mAbilityTimer);
		TheApplication.getRoot()->removeFrameListener(&mProjectileTimer);
		TheApplication.getRoot()->removeFrameListener(&mFirewallTimer);

	}
}