/****************************************************

Because of the length of GLPlayState.cpp
functions related to input have been placed in here.

****************************************************/

#include "GLPlayState.h"
#include "GLMenuState.h"

namespace GAME
{
	// Sets the minimum and maximum height the camera can go.
	static const float minCameraHeight = 200.0f;
	static const float maxCameraHeight = 2500.0f;

	// Sets the minimum and maximum pitch the camera can rotate to.
	static const int minCameraPitch = 0;
	static const int maxCameraPitch = 90;

	// Capture is called every frame. Is used for unbuffered input.
	void PlayState::capture(Ogre::Real time)
	{
		// Sets the keyboard controls
		// W Key, move forward.
		if (m_Input->isKeyDown(OIS::KC_W))
			scrollVertical(-1, time);
		// S Key, move backward.
		if (m_Input->isKeyDown(OIS::KC_S))
			scrollVertical(1, time);
		// A Key, move left.
		if (m_Input->isKeyDown(OIS::KC_A))
			scrollHorizontal(-1, time);
		// D Key, move right.
		if (m_Input->isKeyDown(OIS::KC_D))
			scrollHorizontal(1, time);
		// Q Key, rotate yaw left. Rotates according to World coordinates.
		if (m_Input->isKeyDown(OIS::KC_Q))
			m_CameraSceneNode->yaw(Ogre::Degree(45.0f*time), Ogre::Node::TS_WORLD);
		// E Key, rotate yaw right.
		if (m_Input->isKeyDown(OIS::KC_E))
			m_CameraSceneNode->yaw(Ogre::Degree(-45.0f*time), Ogre::Node::TS_WORLD);

		// Sets mouse controls
		if(!m_MMouseDown)
		{
			CEGUI::Point mousePosition = CEGUI::MouseCursor::getSingleton().getPosition();

			// If the mouse cursor if at the left edge of the screen, scroll left.
			if(mousePosition.d_x <= 0)
				scrollHorizontal(-1, time);
			// If the mouse cursor if at the right edge of the screen, scroll right.
			else if(mousePosition.d_x >= m_ViewPort->getActualWidth() - 1)
				scrollHorizontal(1, time);
			// If the mouse cursor if at the top edge of the screen, scroll up.
			if(mousePosition.d_y <= 0)
				scrollVertical(-1, time);
			// If the mouse cursor if at the bottom edge of the screen, scroll down.
			else if(mousePosition.d_y >= m_ViewPort->getActualHeight() - 1)
				scrollVertical(1, time);
		}
	}

	// Buffered input. Is called when a key is pressed.
	bool PlayState::keyPressed(const OIS::KeyEvent &e)
	{
		switch(e.key)
		{
		// Esc Key. Show the esc menu.
		case OIS::KC_ESCAPE:
			{
				CEGUI::Window *menuWindow = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/MenuWindow");
				menuWindow->setVisible(!menuWindow->isVisible());
			}
			break;
		// Slash Key (/). Show fps and mouse coordinates.
		case OIS::KC_SLASH:
			showDeveloper();
			break;
		// F1 Key. Shortcut to change the state back to MenuState.
		case OIS::KC_F1:
			ChangeState(GAME::MenuState::getSingletonPtr());
			break;
		// Left Shift Key. Doubles the scroll key.
		case OIS::KC_LSHIFT:
			scrollSpeed *= 2;
			break;
		// Pause Key. Pauses the game.
		case OIS::KC_PAUSE:
			setPaused(!getPaused());
			break;
		// M Key. Mutes/UnMutes all game audio.
		case OIS::KC_M:
			SoundManager::getSingletonPtr()->MuteSound();
			break;
		// 1 Key.
		case OIS::KC_1:
			if(!fireballActivated)
				m_ActionType = FIREBALL_ACTION;
			break;
		// 2 Key.
		case OIS::KC_2:
			// Slow
			if(!slowActivated)
			{
				getGame()->slowUnits(1, 0.5f);
				actionActivated(SLOW_ACTION);
			}
			break;
		// 3 Key.
		case OIS::KC_3:
			// Buff
			if(!buffActivated)
			{
				getGame()->buffUnits(1, 0.5f);
				actionActivated(BUFF_ACTION);
			}
			break;
		// 4 Key.
		case OIS::KC_4:
			if(!spawnActivated)
			{
				getGame()->spawnUnit();
				actionActivated(SPAWN_ACTION);
			}
			break;
		// 5 Key.
		case OIS::KC_5:
			if(!teleportActivated)
				m_ActionType = TELEPORT_ACTION;
			break;
		// 6 Key.
		case OIS::KC_6:
			if(!confusedActivated)
				m_ActionType = CONFUSE_ACTION;
			break;
		// 7 Key.
		case OIS::KC_7:
			if(!stealthActivated)
				m_ActionType = STEALTH_ACTION;
			break;
		// 8 Key.
		case OIS::KC_8:
			if(!firewallActivated)
				firewallAction();
			break;
		// ` Key.
		case OIS::KC_GRAVE:
			m_ActionType = DEFAULT_ACTION;
			if(m_TargetedUnit)
			{
				m_TargetedUnit->sceneNode->showBoundingBox(false);
				m_TargetedUnit = NULL;
			}
			if(m_FirewallBoxSceneNode->isInSceneGraph())
				m_SceneMgr->getRootSceneNode()->removeChild(m_FirewallBoxSceneNode);
			popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), BASE, 0, false);
			break;
		// Z Key. Hides the GUI.
		case OIS::KC_Z:
			TheApplication.getGUI()->hideGUI();
		default:
			break;
		}
		return true;
	}
	
	// Buffered input. Is called when a key is release.
	bool PlayState::keyReleased(const OIS::KeyEvent &e)
	{
		switch(e.key)
		{
		// Left Shift Key. Halves the scroll speed.
		case OIS::KC_LSHIFT:
			scrollSpeed /= 2;
			break;
		default:
			break;
		}
		return true;
	}

	// Inline function that determines if a position, unit, is in a circle at location 
	//  with radius. Returns true if unit is in the radius of location.
	inline bool isInRadius(Ogre::Vector3 unit, Ogre::Vector3 location, Ogre::Real radius)
	{
		// Squared distance is used to avoid a square root.
		if( unit.squaredDistance(location) < (radius*radius) )
			return true;
		else
			return false;
	}

	// Buffered input. Is called when a mouse button is pressed.
	bool PlayState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		switch(id)
		{
		// Left Mouse Button.
		case OIS::MB_Left:
			{
				// What happens is dependant upon what the current action is.
				/****************
				 This is a poor solution as it uses a lot of duplicated, and potentially unecessary code.
				 Each RaySceneQuery should use the same code instead of repeating it as needed.
				****************/
				switch(m_ActionType)
				{
				// Default action allows that user to select units and locations to see information
				//  about them in the GUI.
				case DEFAULT_ACTION:
					{
						CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
						CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
						// Only create query if the mouse position is not over top of the gui.
						if(!root->getChildAtPosition(mousePos))
						{
							// Creates a Ray in the direction of the camera.
							Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
								mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
							m_RaySceneQuery->setRay(mouseRay);
							m_RaySceneQuery->setSortByDistance(true);
							// Sets the QueryMask to SELECTABLE_MASK, this query mask only
							//  applies to unit scene nodes, and location scene nodes.
							//  So only those are returned in the result.
							m_RaySceneQuery->setQueryMask(SELECTABLE_MASK);

							// Finds all nodes and parts of terrain along the ray.
							Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
							Ogre::RaySceneQueryResult::iterator itr = result.begin();

							bool foundLocation = false;
							GAME::UnitController *foundUnit = NULL;

							// Iterates through each result until either a unit controller, a location is found or
							//  there are no more results.
							for(itr; itr != result.end(); itr++)
							{
								// Find Unit
								if(itr->movable	&& itr->movable->getParentSceneNode()->getName().substr(0,4)=="Unit")
								{
									// Compares the scene node with each unit's scene node. Returns either the
									//  unit controller or NULL.
									foundUnit = findUnitControllerFromSceneNode(itr->movable->getParentSceneNode());
									setSelectedUnit(foundUnit);
									break;
								}
								// Find Location
								else if(itr->movable && itr->movable->getName().substr(0,4)=="Loca")
								{
									// Compares the scene node with each locations's scene node. Returns either the
									// true or false.
									foundLocation = findLocationFromSceneNode(itr->movable->getParentSceneNode());
									break;
								}

							//	break;
							}
							// If a unit was not found. Set m_SelectedUnit = NULL;
							if(!foundUnit)
								setSelectedUnit(NULL);
							// If a location was not found, hide popupWindow and set it to look at the user's
							//  home base.
							if(!foundLocation)
								popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), BASE, 0, false);
						}
					}
					break;
				// Fireball Action creates a fireball at the location of the mouse click.
				case FIREBALL_ACTION:
					{
						// This really shouldn't be here.
						float FireballRadius = 75;
						int   FireballDamage = 60;
						// Creates a SphereSceneQuery at the mouse position with radius 75.
						Ogre::Sphere fireball(m_RayPosition, FireballRadius);
						Ogre::SphereSceneQuery *fireballQuery;

						fireballQuery = m_SceneMgr->createSphereQuery(fireball);
						// Only unit's scene nodes are returned.
						fireballQuery->setQueryMask(UNIT_MASK);

						Ogre::SceneQueryResult &result = fireballQuery->execute();
						Ogre::SceneQueryResultMovableList::iterator itr = result.movables.begin();

						for(itr; itr != result.movables.end(); ++itr)
						{
							GAME::UnitController *foundUnit = NULL;
							if((*itr)->getParentSceneNode()->getName().substr(0,4)=="Unit")
							{
								// For each unit found in the sphere scene query damage it 60 points.
								foundUnit = findUnitControllerFromSceneNode((*itr)->getParentSceneNode());
								if(foundUnit)
									foundUnit->damage(FireballDamage);
							}
						}
						m_SceneMgr->destroyQuery(fireballQuery);
					}
					
					// Create a fireball particle system at ray position.
					m_ParticleSystemManager.createParticleSystem(m_RayPosition, FIREBALL);

					// Fireball has been activated
					actionActivated(FIREBALL_ACTION);

					// Action type now revert to default.
					m_ActionType = DEFAULT_ACTION;
					break;
				// Should not occur.
				case SLOW_ACTION:
					m_ActionType = DEFAULT_ACTION;
					break;
				// Should not occur.
				case BUFF_ACTION:				
					m_ActionType = DEFAULT_ACTION;
					break;
				// Should not occur.
				case SPAWN_ACTION:
					m_ActionType = DEFAULT_ACTION;
					break;
				case TELEPORT_ACTION:
					// If no targeted unit. First stage of teleport action.
					if(!m_TargetedUnit)
					{
						CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
						CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
						// Only create query if the mouse position is not over top of the gui.
						if(!root->getChildAtPosition(mousePos))
						{
							// Creates a Ray in the direction of the camera.
							Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
								mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
							m_RaySceneQuery->setRay(mouseRay);
							m_RaySceneQuery->setSortByDistance(true);
							// Only unit's scene nodes are returned.
							m_RaySceneQuery->setQueryMask(UNIT_MASK);

							Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
							Ogre::RaySceneQueryResult::iterator itr = result.begin();

							GAME::UnitController *foundUnit = NULL;

							// Iterates through each result until either a unit controller is found or
							//  there are no more results.
							for(itr; itr != result.end(); itr++)
							{
							// Find Unit
								if(itr->movable && itr->movable->getParentSceneNode()->getName().substr(0,4)=="Unit")
								{
									// Compares the scene node with each unit's scene node. Returns either the
									//  unit controller or NULL.
									m_TargetedUnit = findUnitControllerFromSceneNode(itr->movable->getParentSceneNode());
									break;
								}
							}
							// If unit was found (m_TargetedUnit), show it's bounding box.
							if(m_TargetedUnit)
								m_TargetedUnit->sceneNode->showBoundingBox(true);
							// Else set actionType to default.
							else
								m_ActionType = DEFAULT_ACTION;
						}
					}
					// If a targeted unit. Second stage of teleport action.
					else
					{
						CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
						CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
						// Only create query if the mouse position is not over top of the gui.
						if(!root->getChildAtPosition(mousePos))
						{
							// Creates a Ray in the direction of the camera.
							Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
								mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
							m_RaySceneQuery->setRay(mouseRay);
							m_RaySceneQuery->setSortByDistance(true);
							// Only unit's scene nodes and wall scene nodes are returned.
							m_RaySceneQuery->setQueryMask(UNIT_MASK | WALL_MASK);

							Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
							Ogre::RaySceneQueryResult::iterator itr = result.begin();

							if(itr != result.end())
							{
								// Checks the first object in the query is not a Wall or Unit, then it must be an acceptable
								//  place to teleport the unit too.
								if( itr->movable && itr->movable->getParentSceneNode()->getName().substr(0,4)!="Wall"
									&& itr->movable->getParentSceneNode()->getName().substr(0,4)!="Unit" )
								{
									// Determine the teleport position.
									Ogre::Vector3 teleportPostion(m_RayPosition.x,m_TargetedUnit->getPosition().y,m_RayPosition.z);
									// Teleport the targeted unit to the teleport position.
									getGame()->getCommanderList()->at(m_TargetedUnit->getCommanderNumber()-1)->teleportUnit
										(m_TargetedUnit, teleportPostion);
									// Teleport action has been activated.
									actionActivated(TELEPORT_ACTION);
								}
							}
						}
						
						// Hide the bounding box.
						m_TargetedUnit->sceneNode->showBoundingBox(false);
						m_TargetedUnit = NULL;

						m_ActionType = DEFAULT_ACTION;
					}
					break;
				case CONFUSE_ACTION:
					{
						CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
						CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
						// Only create query if the mouse position is not over top of the gui.
						if(!root->getChildAtPosition(mousePos))
						{
							// Creates a Ray in the direction of the camera.
							Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
								mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
							m_RaySceneQuery->setRay(mouseRay);
							m_RaySceneQuery->setSortByDistance(true);
							// Only unit's scene nodes are returned.
							m_RaySceneQuery->setQueryMask(UNIT_MASK);

							Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
							Ogre::RaySceneQueryResult::iterator itr = result.begin();

							GAME::UnitController *foundUnit = NULL;

							// Iterates through each result until either a unit controller is found or
							//  there are no more results.
							for(itr; itr != result.end(); itr++)
							{
								// Find Unit
								if(itr->movable && itr->movable->getParentSceneNode()->getName().substr(0,4)=="Unit")
								{
									m_TargetedUnit = findUnitControllerFromSceneNode(itr->movable->getParentSceneNode());
									break;
								}
							}
							// If a unit was found and it is not a friendly unit.
							if(m_TargetedUnit && m_TargetedUnit->getCommanderNumber() != 1)
							{
								// Confuse the unit.
								getGame()->getCommanderList()->at(m_TargetedUnit->getCommanderNumber()-1)->confuseUnit
									(m_TargetedUnit, 1);
								m_TargetedUnit = NULL;
								// Confuse action activated.
								actionActivated(CONFUSE_ACTION);
							}
						}
					}
					m_ActionType = DEFAULT_ACTION;
					break;
				case STEALTH_ACTION:
					{
						CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
						CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
						// Only create query if the mouse position is not over top of the gui.
						if(!root->getChildAtPosition(mousePos))
						{
							// Creates a Ray in the direction of the camera.
							Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
								mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
							m_RaySceneQuery->setRay(mouseRay);
							m_RaySceneQuery->setSortByDistance(true);
							// Only unit's scene nodes are returned.
							m_RaySceneQuery->setQueryMask(UNIT_MASK);

							Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
							Ogre::RaySceneQueryResult::iterator itr = result.begin();

							GAME::UnitController *foundUnit = NULL;

							// Iterates through each result until either a unit controller is found or
							//  there are no more results.
							for(itr; itr != result.end(); itr++)
							{
								// Find Unit
								if(itr->movable && itr->movable->getParentSceneNode()->getName().substr(0,4)=="Unit")
								{
									// Compares the scene node with each unit's scene node. Returns either the
									//  unit controller or NULL.
									m_TargetedUnit = findUnitControllerFromSceneNode(itr->movable->getParentSceneNode());
									break;
								}
							}
							// If unit was found.
							if(m_TargetedUnit)
							{
								// Hide the unit.
								getGame()->getCommanderList()->at(m_TargetedUnit->getCommanderNumber()-1)->hideUnit(
									m_TargetedUnit);
								m_TargetedUnit = NULL;
								// Stealth action activated.
								actionActivated(STEALTH_ACTION);
							}
						}
					}
					m_ActionType = DEFAULT_ACTION;
					break;
				case FIREWALL_ACTION:
					// Hides the mouse cursor.
					CEGUI::MouseCursor::getSingleton().hide();
					// stores the orientation before the mouse button is released.
					orientationBeforeRelease = m_FirewallBoxSceneNode->getOrientation();
					break;
				default:
					m_ActionType = DEFAULT_ACTION;
					break;
				}
				m_LMouseDown = true;
			}
			break;
		// Right Mouse Button.
		case OIS::MB_Right:
			m_ActionType = DEFAULT_ACTION;
			if(m_TargetedUnit)
			{
				m_TargetedUnit->sceneNode->showBoundingBox(false);
				m_TargetedUnit = NULL;
			}
			// Remove firewallscenebox from the scene graph if it is in it.
			if(m_FirewallBoxSceneNode->isInSceneGraph())
				m_SceneMgr->getRootSceneNode()->removeChild(m_FirewallBoxSceneNode);
			popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), BASE, 0, false);
			m_RMouseDown = true;
			break;
		case OIS::MB_Middle:
			// Hide popup window.
			popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), BASE, 0, false);
			// Hide cursor.
			CEGUI::MouseCursor::getSingleton().hide();
			m_MMouseDown = true;
		default:
			break;
		}
		return true;
	}

	// Buffered input. Is called when a mouse button is released.
	bool PlayState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		switch(id)
		{
		// Left Mouse Button.
		case OIS::MB_Left:
			{
			CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
			CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
			// If mouse cursor is not over top of the GUI.
			if(!root->getChildAtPosition(mousePos))
			{
				switch(m_ActionType)
				{
				// If firewall action.
				case (FIREWALL_ACTION):
					// If the orientation of the firewall scene node is about the same when the mouse button is
					//  released as it was when it was pressed, the firewall will activate.
					if( orientationBeforeRelease.equals(m_FirewallBoxSceneNode->getOrientation(), Ogre::Degree(5.0f)) )
					{
						m_ActionType = DEFAULT_ACTION;
						// Remove firewall scene node from the scene graph.
						if(m_FirewallBoxSceneNode->isInSceneGraph())
							m_SceneMgr->getRootSceneNode()->removeChild(m_FirewallBoxSceneNode);
						// Creates a firewall at the ray position and at the current orientation.
						Ogre::Vector3 position = m_RayPosition;
						position.y = 10.0f;
						getGame()->createFirewall(position, m_FirewallBoxSceneNode->getOrientation());
						// Create a Firewall particle system.
						m_ParticleSystemManager.createParticleSystem(m_RayPosition, FIREWALL);
						// Firewall activated.
						actionActivated(FIREWALL_ACTION);
					}
					// Show mouse cursor.
					CEGUI::MouseCursor::getSingleton().show();
					break;
				default:
					break;
				}
			}
			m_LMouseDown = false;
			}
			break;
		case OIS::MB_Right:
			m_RMouseDown = false;
			break;
		case OIS::MB_Middle:
			m_MMouseDown = false;
			break;
		default:
			break;
		}

		return true;
	}

	// Buffered input. Is called when a mouse button is moved.
	bool PlayState::mouseMoved(const OIS::MouseEvent &e)
	{
		// If the middle mouse button is not down and a firewall isn't being reoriented.
		if(!m_MMouseDown && !(m_LMouseDown && m_ActionType == FIREWALL_ACTION) )
		{
			CEGUI::Window *root = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE");
			CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
			// Only create query if the mouse position is not over top of the GUI.
			if(!root->getChildAtPosition(mousePos))
			{
				// Creates a Ray in the direction of the camera.
				Ogre::Ray mouseRay = m_Camera->getCameraToViewportRay(
					mousePos.d_x/float(e.state.width), mousePos.d_y/float(e.state.height));
				m_RaySceneQuery->setRay(mouseRay);
				m_RaySceneQuery->setSortByDistance(true);
				m_RaySceneQuery->setQueryMask(SELECTABLE_MASK);

				Ogre::RaySceneQueryResult &result = m_RaySceneQuery->execute();
				Ogre::RaySceneQueryResult::iterator itr = result.begin();

				if(itr != result.end())
				{
					if(itr->movable)
					{
						// Calculate the position where the ray first intersects with a node.
						Ogre::Vector3 rayDirection = mouseRay.getDirection();
						rayDirection.normalise();
						m_RayPosition = rayDirection * itr->distance + m_CameraSceneNode->getPosition();

						// Sets the position of the firewallbox to to ray position.
						if(m_ActionType == FIREWALL_ACTION)
						{
							if(!m_FirewallBoxSceneNode->isInSceneGraph())
								m_SceneMgr->getRootSceneNode()->addChild(m_FirewallBoxSceneNode);
							m_FirewallBoxSceneNode->setPosition(m_RayPosition.x,10.0f,m_RayPosition.z);
						}

					}
					// If the node belongs to a unit controller.
					if(itr->movable && itr->movable->getParentSceneNode()->getName().substr(0,4)=="Unit")
					{
						Ogre::SceneNode *newObject = itr->movable->getParentSceneNode();						
						if(m_CurrentObject != newObject)
						{
							// Hides detection and combat circles.
							showCircles(false);
						}
						m_CurrentObject = newObject;
						// Shows detection and combat circles.
						showCircles(true);
						if(m_ActionType == TELEPORT_ACTION || m_ActionType == CONFUSE_ACTION || m_ActionType == STEALTH_ACTION)
						{
							// Show arrow above the unit.
							Ogre::Vector3 position = m_CurrentObject->getPosition();
							position.y = 125.0f;
							m_ArrowSceneNode->setPosition(position);
							if(!m_ArrowSceneNode->isInSceneGraph() )
								m_SceneMgr->getRootSceneNode()->addChild(m_ArrowSceneNode);
						}
					}
					else if(m_CurrentObject)
					{
						showCircles(false);
						m_CurrentObject = NULL;
						if(m_ArrowSceneNode->isInSceneGraph() )
							m_SceneMgr->getRootSceneNode()->removeChild(m_ArrowSceneNode);
					}
				}
			}
			CEGUI::System::getSingleton().injectMouseMove(float(e.state.X.rel), float(e.state.Y.rel));
		}

		// Mouse scroll wheel. Changes the height of the camera.
		if(e.state.Z.rel)
		{
			CEGUI::System::getSingleton().injectMouseWheelChange(e.state.Z.rel / 120.0f);

			// Calculates potential height of camera.
			Ogre::Real currentY = m_CameraSceneNode->getPosition().y;
			Ogre::Real newY = currentY + Ogre::Real(-e.state.Z.rel)/10.0f;
			// If the Camera is in the acceptable range, translate it.
			if(newY > minCameraHeight && newY < maxCameraHeight)
			{
				m_CameraSceneNode->translate(Ogre::Vector3::UNIT_Y * Ogre::Real(-e.state.Z.rel)/6.0f);
			}
			// Reposition the camera to be in the acceptable range.
			else if(newY < minCameraHeight)
			{
				m_CameraSceneNode->setPosition(m_CameraSceneNode->getPosition().x, minCameraHeight, m_CameraSceneNode->getPosition().z);
			}
			else if(newY > maxCameraHeight)
			{
				m_CameraSceneNode->setPosition(m_CameraSceneNode->getPosition().x, maxCameraHeight, m_CameraSceneNode->getPosition().z);
			}
		}
		
		// Left Mouse Button.
		if(m_LMouseDown)
		{
			if(m_ActionType == FIREWALL_ACTION)
			{	// Rotate.
				m_FirewallBoxSceneNode->yaw(Ogre::Degree((float)e.state.X.rel));
			}
		}

		if(m_RMouseDown) { }

		// Middle Mouse Button. Rotate the camera.
		if(m_MMouseDown)
		{
			m_CameraSceneNode->yaw(Ogre::Degree(-e.state.X.rel * 0.1f), Ogre::Node::TS_WORLD);

			// 90 yaw, -90 yaw. Pitch changes from 180 + delta -> 180 - delta
			// Calculates potential height of camera.
			Ogre::Radian yaw = m_CameraSceneNode->getOrientation().getYaw();
			Ogre::Radian originalPitch = m_CameraSceneNode->getOrientation().getPitch(false);
			Ogre::Radian newPitch = Ogre::Degree(-e.state.Y.rel *0.1f) + originalPitch;

			float degrees = newPitch.valueDegrees();

			// The sign of the pitch of the camera reverses depending on the yaw of the camera.
			// Because of this two cases must be checked. If the yaw is <= 90 degrees or > 90 degrees.
			// In both cases the newPitch needs to be tested to ensure it is in the acceptable range.
			if( int(abs(yaw.valueDegrees())) <= 90 &&
				( degrees < -minCameraPitch
				&& degrees > -maxCameraPitch ))
			{
				m_CameraSceneNode->pitch(Ogre::Degree(-e.state.Y.rel * 0.1f));
			}
			else if( int(abs(yaw.valueDegrees())) > 90 &&
				( degrees > maxCameraPitch
				&& degrees < 180.0f - minCameraPitch ))
			{
				m_CameraSceneNode->pitch(Ogre::Degree(-e.state.Y.rel * 0.1f));
			}

			// A check to keep the camera in the acceptable range.
			if( m_CameraSceneNode->getOrientation().getPitch(false).valueDegrees() <= -maxCameraPitch )
			{
				m_CameraSceneNode->pitch(Ogre::Degree(1.0f));
			}

		}
		return true;
	}
}