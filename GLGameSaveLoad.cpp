#include "GLgame.h"

#include "GLPlayState.h"

namespace GAME
{
	bool Game::LoadMap(std::string Filename)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(Filename.c_str());

		tinyxml2::XMLElement *root;
		tinyxml2::XMLElement *element;
		tinyxml2::XMLElement *subelement;
		float distance;
		Ogre::Vector3 position;
		Ogre::Vector3 scale;
		Ogre::Vector2 dimensions;

		root = doc.FirstChildElement("Map");

		// Make Skybox.
		element = root->FirstChildElement("Skybox");
		const char *skyboxmaterialname = element->FirstChildElement("MaterialName")->GetText();
		distance = std::stof(element->FirstChildElement("Distance")->GetText());

		SceneMgr->setSkyBox(true, skyboxmaterialname, distance, true);

		// Make Ground.
		element = root->FirstChildElement("Ground");
		subelement = element->FirstChildElement("Dimensions");
		const char *groundmaterialname = element->FirstChildElement("MaterialName")->GetText();
		dimensions.x = std::stof(subelement->FirstChildElement("X")->GetText());
		dimensions.y = std::stof(subelement->FirstChildElement("Y")->GetText());

		WorldSize.x = dimensions.x;
		WorldSize.y = dimensions.y;

		Ogre::Vector2 segments;
		segments.x = WorldSize.x / 50;
		segments.y = WorldSize.y / 50;

		Ogre::Plane plane(Ogre::Vector3::UNIT_Y, Ogre::Real(0));
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			plane, WorldSize.x, WorldSize.y, 20, 20, true, 1, segments.x, segments.y, Ogre::Vector3::UNIT_Z);

		Ogre::Entity *entGround = SceneMgr->createEntity("GroundEntity", "ground");
		SceneMgr->getRootSceneNode()->createChildSceneNode("Ground")->attachObject(entGround);
		entGround->setMaterialName(groundmaterialname);

		// Make Locations.
		element = root->FirstChildElement("Locations")->FirstChildElement();
		
		while(element->FirstChild())
		{
			const char *name = element->FirstChildElement("Name")->GetText();
			const int type = std::stoi(element->FirstChildElement("Type")->GetText());
			LocationType locationtype;
			switch (type)
			{
			case 0:
				locationtype = BASE;
				break;
			case 1:
				locationtype = RESOURCE;
				break;
			default:
				locationtype = OBJECTIVE;
				break;
			}
			subelement = element->FirstChildElement("Position");
			position.x = std::stof(subelement->FirstChildElement("X")->GetText());
			position.y = std::stof(subelement->FirstChildElement("Y")->GetText());
			position.z = std::stof(subelement->FirstChildElement("Z")->GetText());
			subelement = element->FirstChildElement("Dimensions");
			dimensions.x = std::stof(subelement->FirstChildElement("X")->GetText());
			dimensions.y = std::stof(subelement->FirstChildElement("Y")->GetText());
			const int value = std::stoi(element->FirstChildElement("Value")->GetText());

			createLocation(position,dimensions,const_cast<char*>(name),locationtype,value);

			if(element == root->FirstChildElement("Locations")->LastChildElement())
				break;
			else
				element = element->NextSiblingElement();
		}

		// Make Walls.
		element = root->FirstChildElement("Walls")->FirstChildElement();
		while(element->FirstChild())
		{
			const char *name = element->FirstChildElement("Name")->GetText();
			subelement = element->FirstChildElement("Position");
			position.x = std::stof(subelement->FirstChildElement("X")->GetText());
			position.y = std::stof(subelement->FirstChildElement("Y")->GetText());
			position.z = std::stof(subelement->FirstChildElement("Z")->GetText());
			const char* meshname = element->FirstChildElement("Mesh")->GetText();
			subelement = element->FirstChildElement("Scale");
			scale.x = std::stof(subelement->FirstChildElement("X")->GetText());
			scale.y = std::stof(subelement->FirstChildElement("Y")->GetText());
			scale.z = std::stof(subelement->FirstChildElement("Z")->GetText());

			createWall(position,scale,const_cast<char*>(name));

			if(element == root->FirstChildElement("Walls")->LastChildElement())
				break;
			else
				element = element->NextSiblingElement();
		}
		return true;
	}

	tinyxml2::XMLElement *createXMLElement(tinyxml2::XMLDocument *doc, const char *elementName, const char *text)
	{
		tinyxml2::XMLElement *xmlElement;
		tinyxml2::XMLText *xmlText;

		xmlElement = doc->NewElement(elementName);
		xmlText = doc->NewText(text);
		xmlElement->LinkEndChild(xmlText);
		return xmlElement;
	}

	tinyxml2::XMLElement *createXMLElement(tinyxml2::XMLDocument *doc, const char *elementName, int value)
	{
		tinyxml2::XMLElement *xmlElement;
		tinyxml2::XMLText *xmlText;
		std::stringstream ss;

		ss << value;
		xmlElement = doc->NewElement(elementName);
		xmlText = doc->NewText(ss.str().c_str());
		xmlElement->LinkEndChild(xmlText);
		return xmlElement;
	}

	tinyxml2::XMLElement *createXMLElement(tinyxml2::XMLDocument *doc, const char *elementName, float value)
	{
		tinyxml2::XMLElement *xmlElement;
		tinyxml2::XMLText *xmlText;
		std::stringstream ss;

		ss << value;
		xmlElement = doc->NewElement(elementName);
		xmlText = doc->NewText(ss.str().c_str());
		xmlElement->LinkEndChild(xmlText);
		return xmlElement;
	}

	bool Game::SaveMap(std::string Filename)
	{
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLElement *root;
		tinyxml2::XMLElement *element;
		tinyxml2::XMLElement *subelement;
		Ogre::Vector3 vector3;
		Ogre::Vector2 vector2;
		
		doc.LinkEndChild(doc.NewDeclaration());

		root = doc.NewElement("Map");
		doc.LinkEndChild(root);

		// Name
		tinyxml2::XMLElement *name = doc.NewElement("Name");
		{
			tinyxml2::XMLText *xmlText;
			xmlText = doc.NewText("TestMap2");
			name->LinkEndChild(xmlText);
		}
		root->LinkEndChild(name);

		// Description
		tinyxml2::XMLElement *description = doc.NewElement("Description");
		{
			tinyxml2::XMLText *xmlText;
			xmlText = doc.NewText("This is another test Map.");
			description->LinkEndChild(xmlText);
		}
		root->LinkEndChild(description);

		// Skybox
		tinyxml2::XMLElement *skybox = doc.NewElement("Skybox");
		{
			// NEED MATERIAL NAME (WILL BE DONE IN LEVEL EDITOR)
			skybox->LinkEndChild(createXMLElement(&doc, "MaterialName", "Examples/CloudyNoonSkyBox"));

			skybox->LinkEndChild(createXMLElement(&doc, "Distance", SceneMgr->getSkyBoxGenParameters().skyBoxDistance));
		}
		root->LinkEndChild(skybox);

		// Ground
		tinyxml2::XMLElement *ground = doc.NewElement("Ground");
		{
			element = doc.NewElement("Dimensions");
			//vector3 = SceneMgr->getEntity("GroundEntity")->getBoundingBox().getSize();
			vector2 = WorldSize;
			ground->LinkEndChild(element);
			{
				element->LinkEndChild(createXMLElement(&doc, "X", vector2.x));
				element->LinkEndChild(createXMLElement(&doc, "Y", vector2.y));
			}

			std::string materialname = SceneMgr->getEntity("GroundEntity")->getSubEntity(0)->getMaterialName();
			ground->LinkEndChild(createXMLElement(&doc, "MaterialName", materialname.c_str()));
		}
		root->LinkEndChild(ground);
		
		// Locations
		tinyxml2::XMLElement *locations = doc.NewElement("Locations");
		{
			std::deque<GAME::Location> locationList;
			locationList.insert(locationList.begin(), resourceList.begin(), resourceList.end());
			locationList.insert(locationList.end(), objectiveList.begin(), objectiveList.end());
			locationList.insert(locationList.end(), baseList.begin(), baseList.end());
			char* locationtype = "ERROR";
			std::deque<GAME::Location>::iterator itr;
			for(itr = locationList.begin(); itr != locationList.end(); itr++)
			{
				if(itr->type == OBJECTIVE)
					locationtype = "Objective";
				else if(itr->type == RESOURCE)
					locationtype = "Resource";
				else
					locationtype = "Base";
				element = doc.NewElement(locationtype);
				{
					element->LinkEndChild(createXMLElement(&doc, "Name", itr->mSceneNode->getName().c_str()));

					element->LinkEndChild(createXMLElement(&doc, "Type", itr->type));

					subelement = doc.NewElement("Position");
					element->LinkEndChild(subelement);
					vector3 = itr->getPosition();
					{
						subelement->LinkEndChild(createXMLElement(&doc, "X", vector3.x));
						subelement->LinkEndChild(createXMLElement(&doc, "Y", vector3.y));
						subelement->LinkEndChild(createXMLElement(&doc, "Z", vector3.z));
					}

					subelement = doc.NewElement("Dimensions");
					element->LinkEndChild(subelement);
					vector2 = itr->getDimension();
					{
						subelement->LinkEndChild(createXMLElement(&doc, "X", vector2.x));
						subelement->LinkEndChild(createXMLElement(&doc, "Y", vector2.y));
					}

					element->LinkEndChild(createXMLElement(&doc, "Value", itr->locationValue));
				}
				locations->LinkEndChild(element);

			}		
		}
		root->LinkEndChild(locations);
		
		// Walls
		tinyxml2::XMLElement *walls = doc.NewElement("Walls");
		{
			std::deque<Ogre::SceneNode *>::iterator itr;
			for(itr = wallList.begin(); itr != wallList.end(); itr++)
			{
				element = doc.NewElement("Wall");
				{
					element->LinkEndChild(createXMLElement(&doc, "Name", (*itr)->getName().c_str()));

					subelement = doc.NewElement("Position");
					vector3 = (*itr)->getPosition();
					{
						subelement->LinkEndChild(createXMLElement(&doc, "X", vector3.x));
						subelement->LinkEndChild(createXMLElement(&doc, "Y", vector3.y));
						subelement->LinkEndChild(createXMLElement(&doc, "Z", vector3.z));
					}
					element->LinkEndChild(subelement);

					// Entity name is the same as SceneNode name.
					Ogre::Entity *wall = static_cast<Ogre::Entity*>((*itr)->getAttachedObject(0));
					element->LinkEndChild(createXMLElement(&doc, "Mesh", wall->getMesh()->getName().c_str()));

					subelement = doc.NewElement("Scale");
					vector3 = (*itr)->getScale();
					{
						subelement->LinkEndChild(createXMLElement(&doc, "X", vector3.x));
						subelement->LinkEndChild(createXMLElement(&doc, "Y", vector3.y));
						subelement->LinkEndChild(createXMLElement(&doc, "Z", vector3.z));
					}
					element->LinkEndChild(subelement);
				}
				walls->LinkEndChild(element);
			}
		}
		root->LinkEndChild(walls);

		doc.SaveFile(Filename.c_str());

		Filename.erase(Filename.end()-3,Filename.end());
		Filename = Filename + "png";


		// Set Minimap camera/viewport for image.
		float FOV = 45;
		Ogre::Vector2 worldsize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();
		Ogre::Camera *renderCamera = GAME::PlayState::getSingletonPtr()->getMinimap()->getCamera();
		Ogre::Viewport *v = GAME::PlayState::getSingletonPtr()->getMinimap()->getViewport();
		float cameraHeight = std::max(worldsize.x/2, worldsize.y/2) / std::tan(Ogre::Math::DegreesToRadians(FOV/2));
		renderCamera->setPosition(0,cameraHeight,0);
		renderCamera->lookAt(1,1,1);
		renderCamera->yaw(Ogre::Degree(45.0f));
		renderCamera->setAspectRatio(1);
		renderCamera->setFOVy(Ogre::Degree(FOV));
		v->setOverlaysEnabled(false);
		GAME::PlayState::getSingletonPtr()->getMinimap()->getTexture()->getBuffer()->getRenderTarget()->update();
		GAME::PlayState::getSingletonPtr()->getMinimap()->getTexture()->getBuffer()->getRenderTarget()->writeContentsToFile(Filename.c_str());

		return true;
	}
}