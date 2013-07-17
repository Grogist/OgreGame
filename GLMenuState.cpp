#include "GLMenuState.h"
#include "GLPlayState.h"

namespace GAME
{
	// Refers to the location where to find the game maps.
	static boost::filesystem::path mapPath;
	// Should be: static const boost::filesystem::path mapPath("..\\Map\\"); this gives an
	// access violation error in Debug.

	MenuState MenuState::m_MenuState;

	// Initializes MenuState. Is called whenever the game changes to MenuState.
	void MenuState::Init(void)
	{
		mapPath = "..\\Map\\";
		// Each pointer now points to the relevant data.
		m_GUI = GAME::GUI::getSingletonPtr();
		m_Input = GAME::GameInput::getSingletonPtr();
		m_Root = Ogre::Root::getSingletonPtr();
		// Creates a new SceneManager.
		m_SceneMgr = m_Root->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);
		// Attaches a camera to the SceneManager.
		m_Camera = m_SceneMgr->createCamera("MenuCamera");
		// Creates and attaches the Camera to a SceneNode.
		m_CameraSceneNode = m_SceneMgr->createSceneNode("MenuCamera");
		m_CameraSceneNode->attachObject(m_Camera);
		m_RenderWindow = m_Root->getAutoCreatedWindow();
		m_ViewPort = m_RenderWindow->addViewport(m_Camera);
		m_ViewPort->setBackgroundColour(Ogre::ColourValue(0,0,0));
		m_ViewPort->setOverlaysEnabled(false);
	
		m_Camera->setAspectRatio(Ogre::Real(m_ViewPort->getActualWidth()) / Ogre::Real(m_ViewPort->getActualHeight()));
		m_ShutDown = false;
		m_MapName = " ";
		m_PlayerColour = BLUE;

		// Finds all maps currently available maps and creates references to them in m_MapList.
		// This call must be done before the map ListBox is initialized.
		createMapList();
		// Creates a reference to the CEGUI WindowManager.
		CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
		// Loads the Menu layout.
		CEGUI::Window* MenuInterface = wmgr.loadWindowLayout("GLMenuInterface.layout");
		// Attaches the root Menu window ("INTERFACE") to the GUI root window ("ROOT").
		GAME::GUI::getSingletonPtr()->getSheet()->addChildWindow(MenuInterface);
		// Creates a window in code with my name to ensure that all references to my cannot be removed.
		CEGUI::Window *name = wmgr.createWindow("TaharezLook/StaticText", "MYNAME");
		name->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f,0.0f), CEGUI::UDim(0.0f,0.0f)));
		name->setSize(CEGUI::UVector2(CEGUI::UDim(0.28f,0.0f), CEGUI::UDim(0.05f,0.0f)));
		name->setText("GREGORY LIBERA 2012");
		// Adds my name window to the Credits window.
		wmgr.getWindow("CREDITSINTERFACE")->addChildWindow(name);
		name->show();
		// Sets up the map listbox.
		CEGUI::Listbox* listbox = static_cast<CEGUI::Listbox*>(wmgr.getWindow("Listbox"));
		// Subscribes the maplistselectionfunction to be called whenever the uses selects a new map.
		listbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber(&MenuState::maplistselectionfunction, this));
		listbox->setMultiselectEnabled(false);
		listbox->setAlwaysOnTop(true);
		CEGUI::ListboxTextItem* itemListbox;
		// For each map in m_MapList creates a new ListBoxTextItem to correspond to it.
		for(unsigned int i = 0; i<m_MapList.size(); i++)
		{
			// Loads the xml file from each string in m_MapList.
			tinyxml2::XMLDocument doc;
			doc.LoadFile(m_MapList.at(i).c_str());
			// Finds and temporarily stores the name of each map.
			std::string mapName = doc.FirstChildElement("Map")->FirstChildElement("Name")->GetText();
			// Finds and stores the description of each map.
			m_DescriptionList.push_back(doc.FirstChildElement("Map")->FirstChildElement("Description")->GetText());
			
			// Creates a new ListboxTextItem. I'm not sure if this causes a memory leak as I don't delete
			//  each ListboxTextItem. However the MenuInterface window is destroyed when should delete all
			//  subwindows.
			itemListbox = new CEGUI::ListboxTextItem(mapName, i);
			itemListbox->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
			listbox->addItem(itemListbox);

			// Changes the filename in m_MapList to be the corresponding png file.
			std::string pngFileName = m_MapList.at(i).substr(0, m_MapList.at(i).size()-3);
			pngFileName.append("png");
			std::replace(pngFileName.begin(), pngFileName.end(), '/', '\\');

			// Creates a CEGUI imageset for each map image.
			CEGUI::Imageset *imageset = &CEGUI::ImagesetManager::getSingleton().createFromImageFile(mapName, pngFileName);
			// Numbers are best guess.
			imageset->setNativeResolution(CEGUI::Size(1400,1400));
				
			m_ImageList.push_back(imageset);
		}
		listbox->ensureItemIsVisible(itemListbox);

		// Shows and hides the appropriate windows initially.
		wmgr.getWindow("MENUINTERFACE")->show();
		wmgr.getWindow("CREDITSINTERFACE")->hide();
		wmgr.getWindow("LOADINTERFACE")->hide();
		// Subscribes each button/window to a callback function.
		wmgr.getWindow("MENUINTERFACE/QuitButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::quitfunction, this));
		wmgr.getWindow("MENUINTERFACE/LoadButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::loadmenufunction, this));
		wmgr.getWindow("MENUINTERFACE/CreditsButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::creditsfunction, this));
		wmgr.getWindow("CREDITSINTERFACE/ReturnButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::returnfunction, this));
		wmgr.getWindow("LOADINTERFACE/ReturnButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::returnfunction, this));
		wmgr.getWindow("LOADINTERFACE/LoadGame")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::loadfunction, this));
		wmgr.getWindow("LOADINTERFACE/Blue")->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&MenuState::bluefunction, this));
		wmgr.getWindow("LOADINTERFACE/Red")->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&MenuState::redfunction, this));
		wmgr.getWindow("LOADINTERFACE/Green")->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&MenuState::greenfunction, this));
		wmgr.getWindow("LOADINTERFACE/Dark Blue")->subscribeEvent(CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&MenuState::darkbluefunction, this));
		//m_MapName = "../Map/map1.xml";
		//ChangeState(GAME::PlayState::getSingletonPtr());
	}

	// Is called whenever the game state changes from MenuState.
	// Cleans up no longer needed and obsolete information.
	void MenuState::Cleanup(void)
	{
		m_SceneMgr->clearScene();
		m_SceneMgr->destroyAllCameras();
		m_Root->destroySceneManager(m_SceneMgr);
		m_RenderWindow->removeAllViewports();

		m_GUI = NULL;
		m_Input = NULL;
		m_Root = NULL;
		m_SceneMgr = NULL;
		m_Camera = NULL;
		m_CameraSceneNode = NULL;
		m_RenderWindow = NULL;
		m_ViewPort = NULL;

		m_MapList.clear();
		m_ImageList.clear();

		// Destroys each imageset in m_ImageList.
		for(unsigned int i = 0; i<m_ImageList.size(); i++)
		{
			CEGUI::ImagesetManager::getSingleton().destroy(*m_ImageList.at(i));
		}

		// Destroy the menu window.
		CEGUI::WindowManager::getSingleton().destroyWindow("INTERFACE");
	}

	void MenuState::Pause() { }

	void MenuState::Resume() { }

	void MenuState::capture(Ogre::Real time) { }

	// Is called whenever the user presses a keyboard button.
	bool MenuState::keyPressed(const OIS::KeyEvent &e)
	{
		// Acts according to which key is pressed.
		switch(e.key)
		{
		// Esc button
		case OIS::KC_ESCAPE:
			// Sets the game to end on the next frame.
			m_ShutDown = true;
			break;
		// F1 button
		case OIS::KC_F1:
			// Changes the game state to PlayState.
			if(m_MapName != " ")
				ChangeState(GAME::PlayState::getSingletonPtr());
			break;
		// M button
		case OIS::KC_M:
			// Mutes/UnMutes all sound.
			SoundManager::getSingletonPtr()->MuteSound();
			break;
		default:
			break;
		}
		return true;
	}

	bool MenuState::keyReleased(const OIS::KeyEvent &e)
	{
		return true;
	}

	bool MenuState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	bool MenuState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	// Is called whenever the user moves the mouse.
	bool MenuState::mouseMoved(const OIS::MouseEvent &e)
	{
		// Alerts CEGUI System that the mouse has moved.
		CEGUI::System::getSingleton().injectMouseMove(float(e.state.X.rel), float(e.state.Y.rel));

		return true;
	}

	// Is called once every frame.
	// Returning false means that the game will end, true means that the game will continue.
	//  See GLApplication.h for more.
	bool MenuState::frameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(m_ShutDown)
			return false;

		// Updates the SoundManager.
		SoundManager::getSingletonPtr()->FrameStarted(m_CameraSceneNode,evt.timeSinceLastFrame);

		// Updates the GUI.
		GAME::GUI::getSingletonPtr()->injectTimestamps(evt.timeSinceLastFrame);

		return true;
	}

	void MenuState::injectTimestamps(Ogre::Real timeSinceLastFrame) { }

	// Causes game to end next frame.
	bool MenuState::quitfunction(const CEGUI::EventArgs &e)
	{
		m_ShutDown = true;
		return true;
	}

	// Attempts to change the game state.
	bool MenuState::loadfunction(const CEGUI::EventArgs &e)
	{
		if(m_MapName != " ")
			ChangeState(GAME::PlayState::getSingletonPtr());

		return true;
	}
	
	// Switches the menu to the load map menu.
	bool MenuState::loadmenufunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("MENUINTERFACE")->hide();
		CEGUI::WindowManager::getSingleton().getWindow("CREDITSINTERFACE")->hide();
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE")->show();
		return true;
	}

	// Switches the menu to the credits menu.
	bool MenuState::creditsfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("MENUINTERFACE")->hide();
		CEGUI::WindowManager::getSingleton().getWindow("CREDITSINTERFACE")->show();
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE")->hide();
		return true;
	}

	// Switches the menu to the main menu.
	bool MenuState::returnfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("MENUINTERFACE")->show();
		CEGUI::WindowManager::getSingleton().getWindow("CREDITSINTERFACE")->hide();
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE")->hide();
		return true;
	}

	// Called when the user interacts with the map listbox.
	bool MenuState::maplistselectionfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
		CEGUI::Listbox *listBox = static_cast<CEGUI::Listbox*>(wmgr.getWindow("Listbox"));
		// Gets a reference to the selected item.
		CEGUI::ListboxItem *selectedItem = listBox->getFirstSelectedItem();

		// Gets a references to the minimap window.
		CEGUI::Window *minimapWindow = wmgr.getWindow("LOADINTERFACE/GameMinimap");
		// Gets a references to the description window.
		CEGUI::Window *descriptionWindow = wmgr.getWindow("LOADINTERFACE/GameDescription");
		if(selectedItem)
		{
			// The items ID corresponds to a maps location in m_ImageList, m_DescriptionList,
			//  and m_MapList.
			int selectedItemID = selectedItem->getID();

			// Places the minimap image into the minimap window.
			minimapWindow->setProperty("Image", CEGUI::PropertyHelper::imageToString(
				&m_ImageList.at(selectedItemID)->getImage("full_image")));

			// Changes the description window to be the map description.
			descriptionWindow->setText(m_DescriptionList.at(selectedItemID));

			// Sets m_MapName to the name of the selected map (ie. map1.xml).
			m_MapName = m_MapList.at((int)selectedItem->getID());
		}
		else
		{
			// If no item is selected makes the description window show nothing.
			descriptionWindow->setText(" ");

			// Clears the selected map name.
			m_MapName = " ";
		}
 
		return true;
	}

	// Sets the text colour of the Player Colour window to be blue.
	bool MenuState::bluefunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE/Player Colour")->setText("[colour='FFC5D5E1']Player Colour");
		m_PlayerColour = BLUE;
		return true;
	}

	// Sets the text colour of the Player Colour window to be red.
	bool MenuState::redfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE/Player Colour")->setText("[colour='FFE87171']Player Colour");
		m_PlayerColour = RED;
		return true;
	}

	// Sets the text colour of the Player Colour window to be green.
	bool MenuState::greenfunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE/Player Colour")->setText("[colour='FF00F83F']Player Colour");
		m_PlayerColour = GREEN;
		return true;
	}

	// Sets the text colour of the Player Colour window to be dark blue.
	bool MenuState::darkbluefunction(const CEGUI::EventArgs &e)
	{
		CEGUI::WindowManager::getSingleton().getWindow("LOADINTERFACE/Player Colour")->setText("[colour='FF6F00FF']Player Colour");
		m_PlayerColour = DARKBLUE;
		return true;
	}

	// Creates the list of available maps.
	void MenuState::createMapList(void)
	{
		// List of XML files and PNG files
		std::vector<std::string> m_XML;
		std::vector<std::string> m_PNG;		
		
		// Empty iterator to check is the map folder has been iterated through.
		boost::filesystem::directory_iterator end_iter;
		// The doc refering to an xml file.
		tinyxml2::XMLDocument doc;

		// Checks for path existance and if it is a directory.
		if( boost::filesystem::exists(mapPath) && boost::filesystem::is_directory(mapPath) )
		{
			// Iterates through each file in mapPath.
			for( boost::filesystem::directory_iterator dir_iter(mapPath); dir_iter!=end_iter ; ++dir_iter)
			{
				// Check for a file being "regular", and checks for file being an xml file.
				if( boost::filesystem::is_regular_file(dir_iter->status())
					&& dir_iter->path().string().substr(dir_iter->path().string().size()-3, 3) == "xml")
				{
					// Loads the xml file.
					doc.LoadFile(dir_iter->path().string().c_str());
					// Does nothing if there is an error loading the files.
					if(!doc.ErrorID())
					{
						// Adds the filename to m_XML.
						m_XML.push_back(dir_iter->path().string().c_str());
						// Changes to the correct version of slash used by all systems.
						std::replace(m_XML.back().begin(), m_XML.back().end(), '\\', '/');
					} // tinyxml2 error.
				} // is_regular_file && is xml file.
				// Checks for file being a png image.
				else if( boost::filesystem::is_regular_file(dir_iter->status())
					&& dir_iter->path().string().substr(dir_iter->path().string().size()-3, 3) == "png")
				{
					// Add the filename to m_PNG.
					m_PNG.push_back(dir_iter->path().string().c_str());
					std::replace(m_PNG.back().begin(), m_PNG.back().end(), '\\', '/');
				} // is_regular_file && is png file.

			} //directory_iterator
		} //path exists.
		// Eliminate all paths that do not have an associated png image.
		for(unsigned int xml_Counter = 0; xml_Counter<m_XML.size(); xml_Counter++)
		{
			for(unsigned int png_Counter = 0; png_Counter<m_PNG.size(); png_Counter++)
			{
				// If the filenames (withough filetype) are not the same the map if not added
				//  to m_MapList.
				if( m_XML.at(xml_Counter).substr(0, m_XML.at(xml_Counter).size()-3) == 
					m_PNG.at(png_Counter).substr(0, m_PNG.at(png_Counter).size()-3) )
				{
					m_MapList.push_back(m_XML.at(xml_Counter));
				}
			}
		}
	}
}