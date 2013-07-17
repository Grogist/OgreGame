#include "GLMenuState.h"
#include "GLPlayState.h"

namespace GAME
{
	// Each ability can only be used once every few seconds.
	static const float fireballactionRechargeRate	= 0.1666f;  // 6s
	static const float slowactionRechargeRate		= 0.0666f; // 15s
	static const float buffactionRechargeRate		= 0.0666f; // 15s
	static const float spawnactionRechargeRate		= 0.0666f; // 15s
	static const float teleportactionRechargeRate	= 0.125;   // 8s
	static const float confusedactionRechargeRate	= 0.1666f;  // 6s
	static const float stealthactionRechargeRate	= 0.1666f;  // 6s
	static const float firewallactionRechargeRate	= 0.1f;  // 10s

	PlayState PlayState::m_PlayState;

	PlayState::PlayState() : m_CurrentObject(NULL), m_TargetedUnit(NULL),
		m_ActionType(DEFAULT_ACTION), m_SelectedUnit(NULL), frame(0), m_StoredBaseIndex(0), gamePaused(false),
		fireballActivated(false), slowActivated(false), buffActivated(false), spawnActivated(false),
		teleportActivated(false), confusedActivated(false),	stealthActivated(false), firewallActivated(false)
	{
		m_RayPosition = Ogre::Vector3::ZERO;
		fpsTimer.reset();
		m_MiniMapUpdateTimer.reset();
	}

	PlayState::~PlayState()	{ }

	void PlayState::Init(void)
	{
		// Each pointer now points to the relevant data.
		m_GUI = GAME::GUI::getSingletonPtr();
		m_Input = GAME::GameInput::getSingletonPtr();
		m_Root = Ogre::Root::getSingletonPtr();
		// Creates a new SceneManager.
		m_SceneMgr = m_Root->createSceneManager(Ogre::ST_GENERIC);
		// Attaches a camera to the SceneManager.
		m_Camera = m_SceneMgr->createCamera("GameCamera");
		// Creates and attaches the Camera to a SceneNode.
		m_CameraSceneNode = m_SceneMgr->createSceneNode("GameCamera");
		m_CameraSceneNode->attachObject(m_Camera);
		m_RenderWindow = m_Root->getAutoCreatedWindow();
		m_ViewPort = m_RenderWindow->addViewport(m_Camera);		

		m_ViewPort->setBackgroundColour(Ogre::ColourValue(0,0,0));
		m_ViewPort->setOverlaysEnabled(false);
	
		m_Camera->setAspectRatio(Ogre::Real(m_ViewPort->getActualWidth()) / Ogre::Real(m_ViewPort->getActualHeight()));

		// Sets the initial position of the camera.
		// This needs to be redone.
		Ogre::Real height = 800.0f ;
		Ogre::Real angle = 45.0f;
		Ogre::Real hypotenuse = height / Ogre::Math::Cos(angle);
		Ogre::Real x = hypotenuse * Ogre::Math::Sin(angle);
		Ogre::Real y = hypotenuse * Ogre::Math::Cos(angle);
		m_CameraSceneNode->setPosition(Ogre::Vector3(0,height,0));
		m_CameraSceneNode->yaw(Ogre::Degree(-90.0f));
		m_CameraSceneNode->pitch(Ogre::Degree(-45.0f));

		m_Camera->setNearClipDistance(1);

		// Passes a reference of the scene manager to the game.
		m_TheGame.initSceneManager(m_SceneMgr);
		// If the game is unable to successfully load the map, the game shuts down.
		if(!m_TheGame.createScene(GAME::MenuState::getSingletonPtr()->m_MapName))
			m_ShutDown = true;

		// Must be done after the scene has been created.
		m_Minimap.Init(m_SceneMgr, m_Root, GAME::MenuState::getSingletonPtr()->m_MapName);
		m_Minimap.setCamera(m_TheGame.getWorldSize());

		m_ParticleSystemManager.Init();

		m_ShutDown = false;
		gamePaused = false;
		
		gameWonBy = 0;

		scrollSpeed = 600.0f;

		m_RaySceneQuery = m_SceneMgr->createRayQuery(Ogre::Ray());

		m_LMouseDown = false;
		m_RMouseDown = false;
		m_MMouseDown = false;

		// Creates the targeted unit arrow.
		m_Arrow = m_SceneMgr->createEntity("arrow", "Arrow.mesh");
		m_ArrowSceneNode = m_SceneMgr->createSceneNode("arrow");
		m_ArrowSceneNode->attachObject(m_Arrow);
		m_ArrowSceneNode->scale(8.0f,10.0f,8.0f);
		// Sets the colour of the Arrow to correspond with the player's colour.
		switch(	GAME::MenuState::getSingletonPtr()->m_PlayerColour )
		{
		case BLUE:
			m_Arrow->setMaterialName("Textures/ArrowBlue");
			break;
		case RED:
			m_Arrow->setMaterialName("Textures/ArrowRed");
			break;
		case GREEN:
			m_Arrow->setMaterialName("Textures/ArrowGreen");
			break;
		case DARKBLUE:
			m_Arrow->setMaterialName("Textures/ArrowDarkBlue");
			break;
		}

		// Creates the firewallbox Entity and Scene Node
		m_FirewallBox = m_SceneMgr->createManualObject("FireWallBox");
		createBox(m_FirewallBox);
		m_FirewallBoxSceneNode = m_SceneMgr->createSceneNode("FireWallBox");
		m_FirewallBoxSceneNode->attachObject(m_FirewallBox);

		// Creates a reference to the CEGUI WindowManager.
		CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
		// Loads the Game layout.
		CEGUI::Window* GameInterface = wmgr.loadWindowLayout("GLGameInterface.layout");
		// Attaches the root Game window ("GAMEINTERFACE") to the GUI root window ("ROOT").
		GAME::GUI::getSingletonPtr()->getSheet()->addChildWindow(GameInterface);

		// Creates and gets a reference to the texture from the Minimap camera. This texture is then displayed
		//  as the Minimap.
		CEGUI::Texture &guiTex = GAME::GUI::getSingletonPtr()->getRenderer()->createTexture(getMinimap()->getTexture());

		// If the texture has already been created destroy it.
		// This is necessary when multiple games are played in a row.
		if(CEGUI::ImagesetManager::getSingletonPtr()->isDefined("RRTImageSet"))
			CEGUI::ImagesetManager::getSingletonPtr()->destroy("RRTImageSet");
		
		CEGUI::Imageset &imageSet = CEGUI::ImagesetManager::getSingleton().create("RRTImageSet", guiTex);

		imageSet.defineImage("RTTImage",
						CEGUI::Point(0.0f, 0.0f),
						CEGUI::Size(guiTex.getSize().d_width,
									guiTex.getSize().d_height),
						CEGUI::Point(0.0f, 0.0f));
		// Places the imageset created from the minimap camera as image for the minimap window.
		wmgr.getWindow("GAMEINTERFACE/MiniMap")->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet.getImage("RTTImage")));

		/****************
		 Since the number of commanders changes in each map, as well as their colour, the number of shown
		 commander score and resource windows needs to change. Each commanders colour needs to be assigned
		 when a map is loaded.

		 All commander windows are initially hidden. Depending on the size of the commanderList
		 more commander windows are shown, as they are shown the text is changed to be the correct
		 colour.

		 This is a poor solution as it uses a lot of duplicated, and potentially unecessary code.
		****************/
		wmgr.getWindow("GAMEINTERFACE/C1")->hide();
		wmgr.getWindow("GAMEINTERFACE/C1Score")->hide();
		wmgr.getWindow("GAMEINTERFACE/C1Resource")->hide();
		wmgr.getWindow("GAMEINTERFACE/C2")->hide();
		wmgr.getWindow("GAMEINTERFACE/C2Score")->hide();
		wmgr.getWindow("GAMEINTERFACE/C2Resource")->hide();
		wmgr.getWindow("GAMEINTERFACE/C3")->hide();
		wmgr.getWindow("GAMEINTERFACE/C3Score")->hide();
		wmgr.getWindow("GAMEINTERFACE/C3Resource")->hide();
		wmgr.getWindow("GAMEINTERFACE/C4")->hide();
		wmgr.getWindow("GAMEINTERFACE/C4Score")->hide();
		wmgr.getWindow("GAMEINTERFACE/C4Resource")->hide();

		unsigned int i = m_TheGame.getCommanderList()->size();
		if(i>0)
		{
			m_CommanderScores.push_back(wmgr.getWindow("GAMEINTERFACE/C1Score"));
			m_CommanderResources.push_back(wmgr.getWindow("GAMEINTERFACE/C1Resource"));
			switch (m_TheGame.getCommanderList()->at(0)->getUnitColour())
			{
			case BLUE:
				m_CommanderTextColours.push_back("[colour='FFC5D5E1']");
				break;
			case RED:
				m_CommanderTextColours.push_back("[colour='FFE87171']");
				break;
			case GREEN:
				m_CommanderTextColours.push_back("[colour='FF00F83F']");
				break;
			case DARKBLUE:
				m_CommanderTextColours.push_back("[colour='FF6F00FF']");
				break;
			default:
				m_CommanderTextColours.push_back("[colour='FF000000']");
				break;
			}
			wmgr.getWindow("GAMEINTERFACE/C1")->show();
			wmgr.getWindow("GAMEINTERFACE/C1")->setProperty("TextColours", "tl:"+m_CommanderTextColours.back().substr(9,8)+" tr:"+m_CommanderTextColours.back().substr(9,8)+" bl:"+m_CommanderTextColours.back().substr(9,8)+" br:"+m_CommanderTextColours.back().substr(9,8));
			wmgr.getWindow("GAMEINTERFACE/C1Score")->show();
			wmgr.getWindow("GAMEINTERFACE/C1Resource")->show();
		}
		if(i>1)
		{
			m_CommanderScores.push_back(wmgr.getWindow("GAMEINTERFACE/C2Score"));
			m_CommanderResources.push_back(wmgr.getWindow("GAMEINTERFACE/C2Resource"));
			switch (m_TheGame.getCommanderList()->at(1)->getUnitColour())
			{
			case BLUE:
				m_CommanderTextColours.push_back("[colour='FFC5D5E1']");
				break;
			case RED:
				m_CommanderTextColours.push_back("[colour='FFE87171']");
				break;
			case GREEN:
				m_CommanderTextColours.push_back("[colour='FF00F83F']");
				break;
			case DARKBLUE:
				m_CommanderTextColours.push_back("[colour='FF6F00FF']");
				break;
			default:
				m_CommanderTextColours.push_back("[colour='FF000000']");
				break;
			}
			wmgr.getWindow("GAMEINTERFACE/C2")->show();
			wmgr.getWindow("GAMEINTERFACE/C2")->setProperty("TextColours", "tl:"+m_CommanderTextColours.back().substr(9,8)+" tr:"+m_CommanderTextColours.back().substr(9,8)+" bl:"+m_CommanderTextColours.back().substr(9,8)+" br:"+m_CommanderTextColours.back().substr(9,8));
			wmgr.getWindow("GAMEINTERFACE/C2Score")->show();
			wmgr.getWindow("GAMEINTERFACE/C2Resource")->show();
		}
		if(i>2)
		{
			m_CommanderScores.push_back(wmgr.getWindow("GAMEINTERFACE/C3Score"));
			m_CommanderResources.push_back(wmgr.getWindow("GAMEINTERFACE/C3Resource"));
			switch (m_TheGame.getCommanderList()->at(2)->getUnitColour())
			{
			case BLUE:
				m_CommanderTextColours.push_back("[colour='FFC5D5E1']");
				break;
			case RED:
				m_CommanderTextColours.push_back("[colour='FFE87171']");
				break;
			case GREEN:
				m_CommanderTextColours.push_back("[colour='FF00F83F']");
				break;
			case DARKBLUE:
				m_CommanderTextColours.push_back("[colour='FF6F00FF']");
				break;
			default:
				m_CommanderTextColours.push_back("[colour='FF000000']");
				break;
			}
			wmgr.getWindow("GAMEINTERFACE/C3")->show();
			wmgr.getWindow("GAMEINTERFACE/C3")->setProperty("TextColours", "tl:"+m_CommanderTextColours.back().substr(9,8)+" tr:"+m_CommanderTextColours.back().substr(9,8)+" bl:"+m_CommanderTextColours.back().substr(9,8)+" br:"+m_CommanderTextColours.back().substr(9,8));
			wmgr.getWindow("GAMEINTERFACE/C3Score")->show();
			wmgr.getWindow("GAMEINTERFACE/C3Resource")->show();
		}
		if(i>3)
		{
			m_CommanderScores.push_back(wmgr.getWindow("GAMEINTERFACE/C4Score"));
			m_CommanderResources.push_back(wmgr.getWindow("GAMEINTERFACE/C4Resource"));
			switch (m_TheGame.getCommanderList()->at(3)->getUnitColour())
			{
			case BLUE:
				m_CommanderTextColours.push_back("[colour='FFC5D5E1']");
				break;
			case RED:
				m_CommanderTextColours.push_back("[colour='FFE87171']");
				break;
			case GREEN:
				m_CommanderTextColours.push_back("[colour='FF00F83F']");
				break;
			case DARKBLUE:
				m_CommanderTextColours.push_back("[colour='FF6F00FF']");
				break;
			default:
				m_CommanderTextColours.push_back("[colour='FF000000']");
				break;
			}
			wmgr.getWindow("GAMEINTERFACE/C4")->show();
			wmgr.getWindow("GAMEINTERFACE/C4")->setProperty("TextColours", "tl:"+m_CommanderTextColours.back().substr(9,8)+" tr:"+m_CommanderTextColours.back().substr(9,8)+" bl:"+m_CommanderTextColours.back().substr(9,8)+" br:"+m_CommanderTextColours.back().substr(9,8));
			wmgr.getWindow("GAMEINTERFACE/C4Score")->show();
			wmgr.getWindow("GAMEINTERFACE/C4Resource")->show();
		}
		
		wmgr.getWindow("GAMEINTERFACE/QuitButton")->hide();
		xCoordinate = wmgr.getWindow("GAMEINTERFACE/Pitch");
		xCoordinate->hide();
		yCoordinate = wmgr.getWindow("GAMEINTERFACE/Yaw");
		yCoordinate->hide();

		priority = wmgr.getWindow("GAMEINTERFACE/PopupWindow/Priority");
		fps = wmgr.getWindow("GAMEINTERFACE/FPS");
		fps->hide();
		unitHealth = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitHealth");
		unitAttack = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitAttack");
		unitDefense = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitDefense");
		unitAttackSpeed = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitAttackSpeed");
		unitMoveSpeed = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitMoveSpeed");
		unitAccuracy = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitAccuracy");
		unitEvasion = wmgr.getWindow("GAMEINTERFACE/UnitWindow/UnitEvasion");
		popup = wmgr.getWindow("GAMEINTERFACE/PopupWindow");
		popup->hide();
		popup->disable();

		wmgr.getWindow("GAMEINTERFACE/MenuWindow")->hide();
		wmgr.getWindow("GAMEINTERFACE/MenuWindow")->setAlwaysOnTop(true);

		wmgr.getWindow("GAMEINTERFACE/GameOverWindow")->hide();

		// Subscribes each button/window to a callback function.
		wmgr.getWindow("GAMEINTERFACE/QuitButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::quitfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Fireball")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::fireballfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Slow")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::slowfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Buff")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::bufffunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Spawn")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::spawnfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Teleport")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::teleportfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Confuse")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::confusefunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Stealth")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::stealthfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Firewall")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::firewallfunction, this));
		wmgr.getWindow("GAMEINTERFACE/ActionsWindow/Default")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::defaultactionfunction, this));
		wmgr.getWindow("GAMEINTERFACE/PopupWindow/LeftArrow")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::leftarrowfunction, this));
		wmgr.getWindow("GAMEINTERFACE/PopupWindow/RightArrow")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::rightarrowfunction, this));
		wmgr.getWindow("GAMEINTERFACE/MenuWindow/Unload")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::unloadfunction, this));
		wmgr.getWindow("GAMEINTERFACE/MenuWindow/Quit")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::quitfunction, this));
		wmgr.getWindow("GAMEINTERFACE/MiniMap")->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&PlayState::minimapclickfunction, this));
	}

	// Is called whenever the game state changes from PlayState.
	// Cleans up no longer needed and obsolete information.
	void PlayState::Cleanup(void)
	{		
		//m_TheGame.SaveMap(GAME::MenuState::getSingletonPtr()->m_MapName);

		m_TheGame.Cleanup();
		m_Minimap.Cleanup();
		m_ParticleSystemManager.Cleanup();

		m_SceneMgr->clearScene();
		m_SceneMgr->destroyAllCameras();
		m_RenderWindow->removeAllViewports();
		m_SceneMgr->destroyQuery(m_RaySceneQuery);
		m_Root->destroySceneManager(m_SceneMgr);
		
		m_CommanderScores.clear();
		m_CommanderResources.clear();
		m_CommanderTextColours.clear();

		m_TargetedUnit = NULL;
		m_SelectedUnit = NULL;
		m_CurrentObject = NULL;

		m_GUI = NULL;
		m_Input = NULL;
		m_Root = NULL;
		m_SceneMgr = NULL;
		m_Camera = NULL;
		m_CameraSceneNode = NULL;
		m_RenderWindow = NULL;
		m_ViewPort = NULL;

		// Destroys the GAMEINTERFACE window.
		CEGUI::WindowManager::getSingleton().destroyWindow("GAMEINTERFACE");

		// Removes and destroys the material used to create the minimap image.
		//Ogre::MaterialManager::getSingleton().remove("MinimapMaterial");		
	}

	void PlayState::Pause() { }

	void PlayState::Resume() { }

	// Is called once every frame.
	// Returning false means that the game will end, true means that the game will continue.
	//  See GLApplication.h for more.
	bool PlayState::frameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		// Causes the game to end.
		if(m_ShutDown)
			return false;

		// Forces the game to be paused if the game is won.
		if(gameWonBy)
		{
			gamePaused = true;
		}

		// Only updates the game is it isn't paused.
		if(!gamePaused)
		{
			// Updates the Game.
			m_TheGame.update(evt);
			// Updates the Minimap every 775 milliseconds.
			if(m_MiniMapUpdateTimer.getMilliseconds() > 775.0f)
			{
				m_Minimap.update();
				m_MiniMapUpdateTimer.reset();
			}

			/****************
			 Since each frame there needs to be a check for whether or not an ability has been activated
			 and since the determining factor for whether or not an ability can be used again is the
			 alpha of the corresponding button this also needs to be checked each from.

			 This is a poor solution as it uses a lot of duplicated, and potentially unecessary code.
			****************/
			CEGUI::ProgressBar *progressBar;
			if(fireballActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Fireball/ProgressBar"));
				if(progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * fireballactionRechargeRate);
				}
				else if(progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Fireball")->setEnabled(true);
					progressBar->setProgress(0.0f);
					fireballActivated = false;
				}
			}			
			if(slowActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Slow/ProgressBar"));
				if(slowActivated && progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * slowactionRechargeRate);
				}
				else if(slowActivated && progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Slow")->setEnabled(true);
					progressBar->setProgress(0.0f);
					slowActivated = false;
				}
			}
			if(buffActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Buff/ProgressBar"));
				if(progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * buffactionRechargeRate);
				}
				else if(progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Buff")->setEnabled(true);
					progressBar->setProgress(0.0f);
					buffActivated = false;
				}
			}
			if(spawnActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Spawn/ProgressBar"));
				if(progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * spawnactionRechargeRate);
				}
				else if(progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Spawn")->setEnabled(true);
					progressBar->setProgress(0.0f);
					spawnActivated = false;
				}
			}
			if(teleportActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Teleport/ProgressBar"));
				if(progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * teleportactionRechargeRate);
				}
				else if(progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Teleport")->setEnabled(true);
					progressBar->setProgress(0.0f);
					teleportActivated = false;
				}
			}
			if(confusedActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Confuse/ProgressBar"));
				if(confusedActivated && progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * confusedactionRechargeRate);
				}
				else if(confusedActivated && progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Confuse")->setEnabled(true);
					progressBar->setProgress(0.0f);
					confusedActivated = false;
				}
			}
			if(stealthActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Stealth/ProgressBar"));
				if(stealthActivated && progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * stealthactionRechargeRate);
				}
				else if(stealthActivated && progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Stealth")->setEnabled(true);
					progressBar->setProgress(0.0f);
					stealthActivated = false;
				}
			}
			if(firewallActivated)
			{
				progressBar = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Firewall/ProgressBar"));
				if(firewallActivated && progressBar->getProgress() < 0.95f)
				{
					progressBar->adjustProgress(evt.timeSinceLastFrame * firewallactionRechargeRate);
				}
				else if(firewallActivated && progressBar->getProgress() >= 0.95f)
				{
					CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Firewall")->setEnabled(true);
					progressBar->setProgress(0.0f);
					firewallActivated = false;
				}
			}
		}

		if( m_ArrowSceneNode->isInSceneGraph() && m_CurrentObject )
		{
			m_ArrowSceneNode->setPosition(m_CurrentObject->getPosition().x, 125.0f, m_CurrentObject->getPosition().z);
		}

		// Legacy function. It is still used for convience.
		injectTimestamps(evt.timeSinceLastFrame);

		// Updates the SoundManager.
		SoundManager::getSingletonPtr()->FrameStarted(m_CameraSceneNode, evt.timeSinceLastFrame);

		return true;
	}

	// Legacy function. It is still used for convience.
	// Mainly updates the rest of the GUI.
	void PlayState::injectTimestamps(Ogre::Real timeSinceLastFrame)
	{
		// Updates the GUI.
		GAME::GUI::getSingletonPtr()->injectTimestamps(timeSinceLastFrame);

		// Recalculates the FPS.
		static const int NUM_FPS_SAMPLES = 64;
		fpsSamples[frame % NUM_FPS_SAMPLES] = 1.0f / timeSinceLastFrame;
		float fp = 0;
		for (int i = 0; i<NUM_FPS_SAMPLES; i++)
		{
			fp+= fpsSamples[i];
		}
		fp /= NUM_FPS_SAMPLES;
		if(frame > 10000)
		{ frame = 0; }
		frame++;

		char windowText[21];
		Ogre::Vector3 position = getRayPosition();
		// Displays the current x,z coordinate of the  first intersection of the mouse ray
		sprintf(windowText,"Mouse X %0.1f", position.x);
		xCoordinate->setText(windowText);		
		sprintf(windowText,"Mouse Z %0.1f", position.z);
		yCoordinate->setText(windowText);

		// Every 675 milliseconds certain information of the game GUI is updated.
		if(fpsTimer.getMilliseconds()>675)
		{
			// Creates a list for each the commander scores and resources.
			std::vector<int> scores = getGame()->getScore();
			std::vector<int> resources = getGame()->getResources();

			// Sets the text for the commander scores and resources in the GUI.
			for(unsigned int i = 0; i<scores.size(); i++)
			{
				char windowText2[21];
				sprintf(windowText2,"%d", scores.at(i));
				std::string text("%d", scores.at(i));
				m_CommanderScores.at(i)->setText(m_CommanderTextColours.at(i) + windowText2);
				sprintf(windowText2,"%d", resources.at(i));
				m_CommanderResources.at(i)->setText(m_CommanderTextColours.at(i) + windowText2);
			}

			// Sets the text for the current FPS.
			sprintf(windowText, "FPS: %0.1f", fp);
			fps->setText(windowText);

			// If the priority popup window is visible get the player's commander's priority for the stored location.
			if(priority->isVisible())
			{
				int	locationPriority = getGame()->getCommanderList()->at(0)->getLocationPriority(m_StoredIndex, m_StoredLocationType);
				sprintf(windowText, "Priority: %i", locationPriority);
				priority->setText(windowText);
			}

			// If a unit is selected.
			if( getSelectedUnit() )
			{
				CEGUI::Window *w = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/UnitWindow/Wireframe");
				w->show();
				// Set the image for the wireframe window to be the image of the appropriate unit type.
				switch( getSelectedUnit()->getUnitType() )
				{
				case UnitType_Fast:
					w->setProperty("Image", "set:Wireframe image:Pyramid");
					break;
				case UnitType_Normal:
					w->setProperty("Image", "set:Wireframe image:Cube");
					break;
				case UnitType_Slow:
					w->setProperty("Image", "set:Wireframe image:Dodecahedron");
					break;
				case UnitType_Ranged:
					w->setProperty("Image", "set:Wireframe image:Octahedron");
					break;
				}

				// Displays the current Health.
				sprintf(windowText, "HP: %d / %d", getSelectedUnit()->getHealth(),
					getSelectedUnit()->getMaxHealth());
				unitHealth->setText(windowText);

				// Displays the current Attack power.
				sprintf(windowText, "Attack: %d", getSelectedUnit()->getAttack());
				unitAttack->setText(windowText);

				// Displays the current Defense power.
				sprintf(windowText, "Defense: %d", getSelectedUnit()->getDefense());
				unitDefense->setText(windowText);

				// Displays the current Attack Speed.
				sprintf(windowText, "A Speed: %.2f", getSelectedUnit()->getAttackSpeed()/1000);
				unitAttackSpeed->setText(windowText);

				// Displays the current Move Speed.
				sprintf(windowText, "Move: %.0f", getSelectedUnit()->getMoveSpeed());
				unitMoveSpeed->setText(windowText);

				// Displays the current Accuracy.
				sprintf(windowText, "Accuracy: %.0f%", getSelectedUnit()->getAccuracy());
				unitAccuracy->setText(windowText);

				// Displays the current Evasion.
				sprintf(windowText, "Evasion: %.0f%", getSelectedUnit()->getEvasion());
				unitEvasion->setText(windowText);
			}
			// If there is not selected unit but there is the popup is visible and a base location is selected.
			else if( m_StoredLocationType == BASE && popup->isVisible() )
			{
				CEGUI::Window *w = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/UnitWindow/Wireframe");
				w->hide();
				// Displays the current Value of the selected location.
				sprintf(windowText, "Value: %d", getGame()->getBaseList()->at(m_StoredIndex).locationValue);
				unitHealth->setText(windowText);
				unitAttack->setText(" ");
				unitDefense->setText(" ");
				unitAttackSpeed->setText(" ");
				unitMoveSpeed->setText(" ");
				unitAccuracy->setText(" ");
				unitEvasion->setText(" ");
			}
			// If there is not selected unit but there is the popup is visible and a objective or resource
			//  location is selected.
			else if( (m_StoredLocationType == OBJECTIVE || m_StoredLocationType == RESOURCE) && popup->isVisible() )
			{
				CEGUI::Window *w = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/UnitWindow/Wireframe");
				w->hide();
				// Displays the current Value of the selected location.
				switch (m_StoredLocationType)
				{				
				case OBJECTIVE:
					sprintf(windowText, "Value: %d", getGame()->getObjectiveList()->at(m_StoredIndex).locationValue);
					break;
				case RESOURCE:
					sprintf(windowText, "Value: %d", getGame()->getResourceList()->at(m_StoredIndex).locationValue);
					break;
				default:
					break;
				}
				unitHealth->setText(windowText);
				unitAttack->setText(" ");
				unitDefense->setText(" ");
				unitAttackSpeed->setText(" ");
				unitMoveSpeed->setText(" ");
				unitAccuracy->setText(" ");
				unitEvasion->setText(" ");
			}
			// If nothing is selected.
			else
			{
				// Display nothing, hide the wireframe window.
				CEGUI::Window *w = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/UnitWindow/Wireframe");
				w->hide();
				unitHealth->setText(" ");
				unitAttack->setText(" ");
				unitDefense->setText(" ");
				unitAttackSpeed->setText(" ");
				unitMoveSpeed->setText(" ");
				unitAccuracy->setText(" ");
				unitEvasion->setText(" ");
			}
		}
	}

	// Returns the current ray position
	Ogre::Vector3 PlayState::getRayPosition(void)
	{
		return m_RayPosition;
	}

	// Returns the current action type
	ActionType PlayState::getActionType(void)
	{
		return m_ActionType;
	}

	// Sets the action type
	void PlayState::setActionType(ActionType actionType)
	{
		m_ActionType = actionType;
	}

	// Sets the selected unit.
	void PlayState::setSelectedUnit(GAME::UnitController *unit)
	{
		m_SelectedUnit = unit;
	}

	// Returns the selected unit.
	GAME::UnitController *PlayState::getSelectedUnit(void)
	{
		return m_SelectedUnit;
	}

	// Checks for conflicts when removing a unit.
	void PlayState::checkUnitPointerConflicts(GAME::UnitController *unit)
	{
		if(unit == m_SelectedUnit)
		{
			m_SelectedUnit = NULL;
		}
		if(unit == m_TargetedUnit)
		{
			m_TargetedUnit = NULL;
		}
		for(unsigned int i = 0; i<m_TheGame.getProjectileList()->size(); i++)
		{
			if(unit == m_TheGame.getProjectileList()->at(i).getTargetEnemy())
			{
				m_TheGame.getProjectileList()->at(i).setTargetEnemy(NULL);
			}
		}
	}

	// When hovering over a unit, show the detection and combat circles.
	void PlayState::showCircles(bool show)
	{
		if(m_CurrentObject)
		{
			// Iterate through each child of the m_CurrentObject scene node. All child will be
			//  either a detection or combat circle node.
			Ogre::SceneNode::ChildNodeIterator children = m_CurrentObject->getChildIterator();
			while(children.hasMoreElements())
			{
				dynamic_cast<Ogre::SceneNode*>(children.getNext())->setVisible(show);
			}
		}
	}

	// Searches through all current unit controllers to check if the scene node associated with them
	//  are the same as the scene node there are being compared against. If it is return that unit
	//  controller. Otherwise return NULL.
	GAME::UnitController *PlayState::findUnitControllerFromSceneNode( Ogre::SceneNode *sceneNode )
	{
		// Find the unit with this sceneNode.
		for(unsigned int commander = 0;
			commander < getGame()->getCommanderList()->size(); commander++)
		{
			GAME::Commander *commanderPtr = getGame()->getCommanderList()->at(commander);
			for(unsigned int unit = 0; unit < commanderPtr->unitList.size(); unit++)
			{
				if(commanderPtr->unitList.at(unit)->sceneNode->getName() == sceneNode->getName())
				{
					return commanderPtr->unitList.at(unit);					
				}

			}
		}
		return NULL;
	}

	// Searches through all locations to check if the scene node associated with them are the
	//  same as the scene node there are being compared against. If found the popup window
	//  appears and a reference to the type and index is stored. Returns true if found, false
	//  if not.
	bool PlayState::findLocationFromSceneNode( Ogre::SceneNode *sceneNode )
	{
		// Find the location with this sceneNode
		unsigned int location;
		// Resource locations.
		if( sceneNode->getName().substr(0,5) == "LocaR" )
		{
			for(location = 0; location < getGame()->getResourceList()->size(); location++)
			{
				if ( sceneNode->getName() == getGame()->getResourceList()->at(location).mSceneNode->getName() )
				{
					// GUI WINDOW POPS UP.
					popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), RESOURCE, location, true);
					return true;
				}
			}
		}
		// Objective locations.
		else if ( sceneNode->getName().substr(0,5) == "LocaO" )
		{
			for(location = 0; location < getGame()->getObjectiveList()->size(); location++)
			{
				if ( sceneNode->getName() == getGame()->getObjectiveList()->at(location).mSceneNode->getName() )
				{
					// GUI WINDOW POPS UP.
					popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), OBJECTIVE, location, true);
					return true;
				}
			}
		}
		// Base locations
		else if ( sceneNode->getName().substr(0,5) == "LocaB" )
		{
			for(location = 0; location < getGame()->getBaseList()->size(); location++)
			{
				if ( sceneNode->getName() == getGame()->getBaseList()->at(location).mSceneNode->getName() )
				{
					// GUI WINDOW POPS UP.
					popupWindow(CEGUI::MouseCursor::getSingleton().getPosition(), BASE, location, true);
					return true;
				}
			}
		}
		return false;
	}

	// Moves the camera scene node forward or backward (it's local z-Axis relative to world
	//  coordinates) according to scrollSpeed, and the modifier.
	// If modifier is -1 move forward, +1 move backward.
	void PlayState::scrollVertical(int modifier, Ogre::Real time)
	{
		Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();

		Ogre::Vector3 moveDirection = quat.zAxis();

		moveDirection.y = 0.0f;
		moveDirection.normalise();

		m_CameraSceneNode->translate(moveDirection * scrollSpeed * Ogre::Real(modifier) * time);
	}

	// Moves the camera scene node left or right (it's local x-Axis) according to scrollSpeed,
	//  and the modifier.
	// If modifier is +1 move right, -1 move left.
	void PlayState::scrollHorizontal(int modifier, Ogre::Real time)
	{
		m_CameraSceneNode->translate(Ogre::Vector3(modifier * scrollSpeed,0.0f,0.0f) * time, Ogre::Node::TS_LOCAL);
	}

	// Places the firewallbox into the scene graph, and places it as the current mouse ray position.
	void PlayState::firewallAction(void)
	{
		m_ActionType = FIREWALL_ACTION;
		if(!m_FirewallBoxSceneNode->isInSceneGraph())
		{
			m_SceneMgr->getRootSceneNode()->addChild(m_FirewallBoxSceneNode);
			m_FirewallBoxSceneNode->setPosition(m_RayPosition.x,10.0f,m_RayPosition.z);
		}
	}

	// Returns the pointer to the Game.
	GAME::Game *PlayState::getGame(void)
	{
		return &m_TheGame;
	}

	// Returns the pointer to the Minimap.
	GAME::Minimap *PlayState::getMinimap(void)
	{
		return &m_Minimap;
	}

	// Returns the pointer to the scene manager.
	Ogre::SceneManager *PlayState::getSceneManager(void)
	{
		return m_SceneMgr;
	}

	// Sets the game to be paused or unpaused.
	void PlayState::setPaused(bool paused)
	{
		gamePaused = paused;
	}

	// Returns whether the game is paused.
	bool PlayState::getPaused(void)
	{
		return gamePaused;
	}

	// Displays the GameOverWindow window and shows who is the winner.
	void PlayState::setGameWonBy(int victor)
	{
		gameWonBy = victor;
		CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/GameOverWindow")->show();
		char vic[5];
		sprintf(vic, "%d", victor);
		CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/GameOverWindow")->setText(m_CommanderTextColours.at(victor-1)+"GAME WON BY COMMANDER "+std::string(vic));
	}

	// Returns how won the game.
	int PlayState::getGameWonBy(void)
	{
		return gameWonBy;
	}

	// Creates a 2-D Box (ie. a rectangle) used for the firewallbox.
	void PlayState::createBox(Ogre::ManualObject *box)
	{
		Ogre::Vector2 boxsize(500.0f, 125.0f);
		box->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);

		box->position(boxsize.x/2, 10.0f, boxsize.y/2);
		box->colour(1,1,1);
		box->position(-boxsize.x/2, 10.0f, boxsize.y/2);
		box->colour(1,1,1);

		box->position(-boxsize.x/2, 10.0f, -boxsize.y/2);
		box->colour(1,1,1);
		box->position(-boxsize.x/2, 10.0f, boxsize.y/2);
		box->colour(1,1,1);

		box->position(-boxsize.x/2, 10.0f, -boxsize.y/2);
		box->colour(1,1,1);
		box->position(boxsize.x/2, 10.0f, -boxsize.y/2);
		box->colour(1,1,1);

		box->position(boxsize.x/2, 10.0f, -boxsize.y/2);
		box->colour(1,1,1);
		box->position(boxsize.x/2, 10.0f, boxsize.y/2);
		box->colour(1,1,1);

		box->end();
	}

	// Sets the game to quit when a quit button is pressed.
	bool PlayState::quitfunction(const CEGUI::EventArgs &e)
	{
		m_ShutDown = true;
		return true;
	}

	// Causes the game to switch to the MenuState.
	bool PlayState::unloadfunction(const CEGUI::EventArgs &e)
	{
		ChangeState(GAME::MenuState::getSingletonPtr());
		return true;
	}

	// In the popup window this decreases the priority of a particular location.
	bool PlayState::leftarrowfunction(const CEGUI::EventArgs &e)
	{
		// commanderList.at(0) is always the player's commander.
		getGame()->getCommanderList()->at(0)->modifyLocationPriority(m_StoredIndex, -1, m_StoredLocationType);
		return true;
	}

	bool PlayState::rightarrowfunction(const CEGUI::EventArgs &e)
	{
		// commanderList.at(0) is always the player's commander.
		getGame()->getCommanderList()->at(0)->modifyLocationPriority(m_StoredIndex, 1, m_StoredLocationType);
		return true;
	}

	// Sets the actionType to fireball.
	bool PlayState::fireballfunction(const CEGUI::EventArgs &e)
	{
		setActionType(FIREBALL_ACTION);
		return true;
	}
	// Activates slowUnits() causing all enemy unit to be slowed.
	bool PlayState::slowfunction(const CEGUI::EventArgs &e)
	{
		getGame()->slowUnits(1, 0.5f);
		actionActivated(SLOW_ACTION);
		return true;
	}
	// Activates buffUnits() causing all friendly unit to be buffed.
	bool PlayState::bufffunction(const CEGUI::EventArgs &e)
	{
		getGame()->buffUnits(1, 0.5f);
		actionActivated(BUFF_ACTION);
		return true;
	}
	// Causes a friendly unit to spawn.
	bool PlayState::spawnfunction(const CEGUI::EventArgs &e)
	{
		getGame()->spawnUnit();
		actionActivated(SPAWN_ACTION);
		return true;
	}
	// Sets the actionType to teleport.
	bool PlayState::teleportfunction(const CEGUI::EventArgs &e)
	{
		setActionType(TELEPORT_ACTION);
		return true;
	}
	// Sets the actionType to confuse.
	bool PlayState::confusefunction(const CEGUI::EventArgs &e)
	{
		setActionType(CONFUSE_ACTION);
		return true;
	}
	// Sets the actionType to stealth.
	bool PlayState::stealthfunction(const CEGUI::EventArgs &e)
	{
		setActionType(STEALTH_ACTION);
		return true;
	}
	// Calls firewallAction().
	bool PlayState::firewallfunction(const CEGUI::EventArgs &e)
	{
		firewallAction();
		return true;
	}
	// Sets the actionType to default.
	bool PlayState::defaultactionfunction(const CEGUI::EventArgs &e)
	{
		setActionType(DEFAULT_ACTION);
		return true;
	}

	// When a user clicks on the minimap move the camera to the relative
	//  position that the user clicked.
	bool PlayState::minimapclickfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();		
		CEGUI::Rect rect = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/MiniMap")->getUnclippedOuterRect();
		
		float x = mousePos.d_x - rect.d_left;
		float y =  mousePos.d_y - rect.d_top;

		float percentY = (x / (rect.d_right - rect.d_left));
		float percentX = 1 - (y / (rect.d_bottom - rect.d_top));

		Ogre::Vector2 worldSize = m_TheGame.getWorldSize();

		Ogre::Vector3 worldPosition;
		worldPosition.x = worldSize.x * percentX - worldSize.x * 0.5f;
		worldPosition.y = m_CameraSceneNode->getPosition().y;
		worldPosition.z = worldSize.y * percentY - worldSize.y * 0.5f;

		m_CameraSceneNode->setPosition(worldPosition);

		return true;
	}

	// Is called whenever an action is activated. Disables the appropriate button, sets it's alpha to 15%, and causes
	//  it's cooldown to occur.
	void PlayState::actionActivated(ActionType actionType)
	{
		switch (actionType)
		{
		case FIREBALL_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Fireball")->setEnabled(false);
			fireballActivated = true;
			break;
		case SLOW_ACTION: 
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Slow")->setEnabled(false);
			slowActivated = true;
			break;
		case BUFF_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Buff")->setEnabled(false);
			buffActivated = true;
			break;
		case SPAWN_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Spawn")->setEnabled(false);
			spawnActivated = true;
			break;
		case TELEPORT_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Teleport")->setEnabled(false);
			teleportActivated = true;
			break;
		case CONFUSE_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Confuse")->setEnabled(false);
			confusedActivated = true;
			break;
		case STEALTH_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Stealth")->setEnabled(false);
			stealthActivated = true;
			break;
		case FIREWALL_ACTION:
			CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/ActionsWindow/Firewall")->setEnabled(false);
			firewallActivated = true;
			break;
		default:
			break;
		}
	}

	// Displays the popupWindow.
	void PlayState::popupWindow(CEGUI::Point mousePosition, LocationType type, int index, bool enable)
	{
		if(enable)
		{
			m_StoredIndex = index;
			m_StoredLocationType = type;
			if(type == BASE)
			{
				m_StoredBaseIndex = index;
			}
			// Repositions the popup window to be located at the current mouse cursor location.
			// Will modify the popup window's position if it would otherwise be cut off by the edge of
			//  the screen.
			CEGUI::Size screenSize = CEGUI::System::getSingleton().getRenderer()->getDisplaySize();
			CEGUI::UVector2 popupPosition;
			float popupHeight = popup->getHeight().d_scale * screenSize.d_height;
			float popupWidth = popup->getWidth().d_scale * screenSize.d_width;
			if(popupHeight < screenSize.d_height)
			{
				float y = (mousePosition.d_y + popupHeight > screenSize.d_height) ? (screenSize.d_height - popupHeight) :
					mousePosition.d_y;
				popupPosition.d_y = cegui_absdim(y);				
			}
			else
			{
				popupPosition.d_y = CEGUI::UDim(0.0f, 0.0f);
			}
			if(popupWidth < screenSize.d_width)
			{
				float x  = (mousePosition.d_x + popupWidth > screenSize.d_width) ? (screenSize.d_width - popupWidth) :
					mousePosition.d_x;
				popupPosition.d_x = cegui_absdim(x);
			}
			else
			{
				popupPosition.d_x = CEGUI::UDim(0.0f,0.0f);
			}

			popup->setPosition(popupPosition);
			popup->moveToFront();

			popup->show();
			popup->enable();
		}
		else
		{
			popup->hide();
			popup->disable();
		}
	}

	// Shows Developer related windows.
	void PlayState::showDeveloper(void)
	{
		xCoordinate->setVisible(!xCoordinate->isVisible());
		yCoordinate->setVisible(!yCoordinate->isVisible());
		fps->setVisible(!fps->isVisible());
		CEGUI::Window *quit = CEGUI::WindowManager::getSingleton().getWindow("GAMEINTERFACE/QuitButton");
		quit->setVisible(!quit->isVisible());
	}
}