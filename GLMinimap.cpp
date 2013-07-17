#include "GLMinimap.h"

#include "GLapplication.h"
#include "GLPlayState.h"

namespace GAME
{
	void Minimap::makeLocationOverlays(std::deque<GAME::Location> *locationList)
	{
		for(unsigned int i = 0; i < locationList->size(); i++)
		{
			std::string s;
			std::stringstream ss;

			ss << numberOfOverLays;

			s = ss.str();
						
			std::pair<Ogre::Overlay*, int*> Overlay;
			Overlay.second = &locationList->at(i).controlledBy;
			Overlay.first = Ogre::OverlayManager::getSingleton().create("OVERLAYNAME" + s);

			Ogre::Vector2 dimension = locationList->at(i).getDimension();
			Ogre::Vector3 position = locationList->at(i).getPosition();

			Ogre::Vector2 worldSize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();

			Ogre::Vector2 offset;
			position.x += worldSize.x/2;
			position.z += worldSize.y/2;
			position.x = position.x/worldSize.x;
			position.z = position.z/worldSize.y;
			offset.x = dimension.x/worldSize.x;
			offset.y = dimension.y/worldSize.y;

			Ogre::OverlayContainer *panel = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","PANELNAME" + s));
			panel->setPosition(0.0f,0.0f);
			panel->setDimensions(1.0f,1.0f);
			Ogre::OverlayElement *left = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","LEFT" + s));
			left->setPosition(position.x - offset.x/2,position.z - offset.y/2 + 0.01f);
			left->setDimensions(0.01f,offset.y);
			left->setMaterialName("BaseWhite");
			panel->addChild(left);
			Ogre::OverlayElement *right = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","RIGHT" + s));
			right->setPosition(position.x + offset.x/2,position.z - offset.y/2 + 0.01f);
			right->setDimensions(0.01f,offset.y);
			right->setMaterialName("BaseWhite");
			panel->addChild(right);
			Ogre::OverlayElement *bottom = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","BOTTOM" + s));
			bottom->setPosition(position.x - offset.x/2 + 0.01f,position.z - offset.y/2);
			bottom->setDimensions(offset.x,0.01f);
			bottom->setMaterialName("BaseWhite");
			panel->addChild(bottom);
			Ogre::OverlayElement *top = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","TOP" + s));
			top->setPosition(position.x - offset.x/2 + 0.01f,position.z + offset.y/2);
			top->setDimensions(offset.x,0.01f);
			top->setMaterialName("BaseWhite");
			panel->addChild(top);
		
			Overlay.first->add2D(panel);
			Overlay.first->hide();
			Overlay.first->rotate(Ogre::Degree(90.0f));
			Overlay.first->setZOrder(1);

			numberOfOverLays++;
			m_LocationOverlayList.push_back(Overlay);
		}
	}

	void Minimap::makeUnitOverlay(GAME::UnitController *unit)
	{
		std::string s;
		std::stringstream ss;

		ss << numberOfOverLays;

		s = ss.str();
						
		std::pair<Ogre::Overlay*, GAME::UnitController*> Overlay;
		Overlay.second = unit;
		Overlay.first = Ogre::OverlayManager::getSingleton().create("OVERLAYNAME" + s);

		Ogre::Vector3 position = unit->getPosition();

		Ogre::Vector2 worldSize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();

		position.x += worldSize.x/2;
		position.z += worldSize.y/2;
		position.x = position.x/worldSize.x;
		position.z = position.z/worldSize.y;

		Ogre::OverlayContainer *panel = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","PANELNAME" + s));
		panel->setPosition(0.0f,0.0f);
		panel->setDimensions(1.0f,1.0f);
		Ogre::OverlayElement *main = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","MAIN" + s));
		main->setPosition(position.x,position.z);
		main->setDimensions(0.015f,0.015f);
		
		// THIS LOOKS HORRIBLE.
		switch (GAME::PlayState::getSingletonPtr()->getGame()->getCommanderList()->at(unit->getCommanderNumber()-1)->getUnitColour())
		{
		case BLUE:
			main->setMaterialName("Overlay/Blue");
			break;
		case DARKBLUE:
			main->setMaterialName("Overlay/DarkBlue");
			break;
		case GREEN:
			main->setMaterialName("Overlay/Green");
			break;
		case RED:
			main->setMaterialName("Overlay/Red");
			break;
		default:
			//main->setMaterialName("BaseWhite");
			break;
		}
		panel->addChild(main);
		
		Overlay.first->add2D(panel);
		Overlay.first->show();
		Overlay.first->rotate(Ogre::Degree(90.0f));
		Overlay.first->setZOrder(2);

		numberOfOverLays++;
		m_UnitOverlayList.push_back(Overlay);		
	}

	void Minimap::removeUnitOverlay(GAME::UnitController *unit)
	{
		for(unsigned int i = 0; i<m_UnitOverlayList.size(); i++)
		{
			if(m_UnitOverlayList.at(i).second == unit)
			{
				Ogre::OverlayManager::getSingleton().destroy(m_UnitOverlayList.at(i).first);
				m_UnitOverlayList.erase(m_UnitOverlayList.begin() + i);
			}
		}
	}

	void Minimap::Init(Ogre::SceneManager *sceneMgr, Ogre::Root *root, std::string mapName)
	{
		tex = root->getTextureManager()->createManual(
			"RTT",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			256,
			256,
			0,
			Ogre::PF_R8G8B8,
			Ogre::TU_RENDERTARGET);

		Ogre::OverlayManager::getSingleton().destroyAll();
		Ogre::OverlayManager::getSingleton().destroyAllOverlayElements();
		Ogre::MaterialManager::getSingleton().remove("MinimapMaterial");
		
		numberOfOverLays = 0;
		makeLocationOverlays(GAME::PlayState::getSingletonPtr()->getGame()->getBaseList());
		makeLocationOverlays(GAME::PlayState::getSingletonPtr()->getGame()->getObjectiveList());
		makeLocationOverlays(GAME::PlayState::getSingletonPtr()->getGame()->getResourceList());

		rtex = tex->getBuffer()->getRenderTarget();
		rtex->setAutoUpdated(false);
		renderCamera = sceneMgr->createCamera("RRTCamera");
		v = rtex->addViewport(renderCamera);
		v->setOverlaysEnabled(true);
		v->setClearEveryFrame(true);
		v->setBackgroundColour(Ogre::ColourValue::Black);
		v->setSkiesEnabled(false);

		Ogre::MaterialPtr m_QuadMaterial = Ogre::MaterialManager::getSingleton().create( "MinimapMaterial" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true );
		Ogre::Pass *pass = m_QuadMaterial->getTechnique( 0 )->getPass( 0 );
		pass->setLightingEnabled(false);
		Ogre::TextureUnitState *tex = pass->createTextureUnitState( "", 0 );
		tex->setTextureName( mapName.substr(0,mapName.size()-3) + "png");
		
		Ogre::Overlay *a = Ogre::OverlayManager::getSingleton().create("MinimapOverlay");
		Ogre::OverlayContainer *container = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","MinimapOverlayContainer"));
		container->setPosition(0.0f,0.0f);
		container->setDimensions(1.0f,1.0f);
		Ogre::OverlayElement *element = static_cast<Ogre::OverlayContainer*>( Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","MinimapOverlayElement"));
		element->setPosition(0.0f,0.0f);
		element->setDimensions(1.0f,1.0f);
		element->setMaterialName("MinimapMaterial");
		container->addChild(element);
		a->add2D(container);
		a->setZOrder(0);
		a->show();
	}

	void Minimap::update(void)
	{
		getTexture()->getBuffer()->getRenderTarget()->update();

		for(unsigned int i = 0; i < m_LocationOverlayList.size(); i++)
		{
			if((*m_LocationOverlayList.at(i).second) != 0)
			{
				Ogre::OverlayContainer *container = 0;
				Ogre::Overlay::Overlay2DElementsIterator itr = m_LocationOverlayList.at(i).first->get2DElementsIterator();
				GAME::UnitColour overlayColour;

				while(itr.hasMoreElements())
				{
					container = itr.getNext();
					Ogre::OverlayContainer::ChildIterator elementItr = container->getChildIterator();
					while(elementItr.hasMoreElements())
					{
						Ogre::OverlayElement *element = elementItr.getNext();
						// THIS LOOKS TERRIBLE
						overlayColour = GAME::PlayState::getSingletonPtr()->getGame()->getCommanderList()->at(*m_LocationOverlayList.at(i).second-1)->getUnitColour();
						if(overlayColour == RED && container && element)
						{							
							element->setMaterialName("Overlay/Red");
						}
						else if(overlayColour == BLUE && container && element)
						{
							element->setMaterialName("Overlay/Blue");
						}
						else if(overlayColour == GREEN && container && element)
						{
							element->setMaterialName("Overlay/Green");
						}
						else if(overlayColour == DARKBLUE && container && element)
						{
							element->setMaterialName("Overlay/DarkBlue");
						}
					}
				}

				m_LocationOverlayList.at(i).first->show();
			}
			else if(m_LocationOverlayList.at(i).first->isVisible())
			{
				m_LocationOverlayList.at(i).first->hide();
			}
		}

		for(unsigned int i = 0; i < m_UnitOverlayList.size(); i++)
		{
			if(m_UnitOverlayList.at(i).second != NULL)
			{
				Ogre::OverlayContainer *container = 0;
				Ogre::Overlay::Overlay2DElementsIterator itr = m_UnitOverlayList.at(i).first->get2DElementsIterator();

				while(itr.hasMoreElements())
				{
					container = itr.getNext();
					Ogre::OverlayContainer::ChildIterator elementItr = container->getChildIterator();
					while(elementItr.hasMoreElements())
					{
						Ogre::OverlayElement *element = elementItr.getNext();
						
						Ogre::Vector3 position = m_UnitOverlayList.at(i).second->getPosition();

						Ogre::Vector2 worldSize = GAME::PlayState::getSingletonPtr()->getGame()->getWorldSize();

						position.x += worldSize.x/2;
						position.z += worldSize.y/2;
						position.x = position.x/worldSize.x;
						position.z = position.z/worldSize.y;

						element->setPosition(position.x,position.z);						
					}
				}
			}
		}
	}

	void Minimap::Cleanup(void)
	{
		m_LocationOverlayList.clear();
		m_UnitOverlayList.clear();

		rtex->removeAllViewports();
	}

	Ogre::TexturePtr Minimap::getTexture(void)
	{
		return tex;
	}

	Ogre::Camera *Minimap::getCamera(void)
	{
		return renderCamera;
	}

	void Minimap::setCamera(Ogre::Vector2 worldsize)
	{
		float FOV = 45;
		// Normal Position
		/*float cameraHeight = std::max(worldsize.x/2, worldsize.y/2) / std::tan(Ogre::Math::DegreesToRadians(FOV/2));
		renderCamera->setPosition(0,cameraHeight,0);
		renderCamera->lookAt(1,1,1);*/
		// Look at nothing position.
		float cameraHeight = -100.0f;
		renderCamera->setPosition(0,cameraHeight,0);
		renderCamera->lookAt(1,-105,1);
		renderCamera->setNearClipDistance(1);
		renderCamera->yaw(Ogre::Degree(45.0f));
		renderCamera->setAspectRatio(1);
		renderCamera->setFOVy(Ogre::Degree(FOV));
	}

	Ogre::Viewport *Minimap::getViewport(void)
	{
		return v;
	}
}