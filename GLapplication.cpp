#include "GLapplication.h"
#include "GLGameState.h"

Application::Application(void) : Root(0), PluginsCfg(Ogre::StringUtil::BLANK), ResourcesCfg(Ogre::StringUtil::BLANK),
	sound(INVALID_SOUND_INDEX), channel(INVALID_SOUND_CHANNEL), playlist(NULL), count(0)
{
}

Application::~Application(void)
{
	while(!m_States.empty())
	{
		m_States.back()->Cleanup();
		m_States.pop_back();
	}

	Ogre::WindowEventUtilities::removeWindowEventListener(Window, this);
	windowClosed(Window);
	delete Root;
	Root = NULL;
}

void Application::init(void)
{
	Window = Root->initialise(true, "RTS");
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	Shutdown = false;
	gamePaused = false;

	soundMgr = GAME::SoundManager::getSingletonPtr();
	soundMgr->Initialize();
	playlist = NULL;
	playlist = soundMgr->CreatePlayList(std::string("playlist.m3u"));
	count = 0;
	// Start playing playlist.
	if(playlist)
	{
        /*
            Get the first song in the playlist, create the sound and then play it.
        */
        result = playlist->getTag("FILE", count, &tag);
        //ERRCHECK(result);
		if (result == FMOD_OK)
		{
			std::string filename;
			filename = (char*)tag.data;

			sound = soundMgr->CreateStream(std::string((char*)tag.data));
			soundMgr->PlaySound(sound,NULL,&channel);

			count++;
		}
	}
}

bool Application::start(GAME::GameState* state)
{
	//Ogre::LogManager::getSingletonPtr()->logMessage("*** Start of Application::Start ***");
	#ifdef _DEBUG
		PluginsCfg = "plugins_d.cfg";
	#else
		PluginsCfg = "plugins.cfg";
	#endif

	#ifdef _DEBUG
		ResourcesCfg = "resources_d.cfg";
	#else
		ResourcesCfg = "resources.cfg";
	#endif

	Root = new Ogre::Root(PluginsCfg);
	Ogre::ConfigFile cf;
	cf.load(ResourcesCfg);

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i=settings->begin(); i!=settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName,typeName,secName);
		}
	}
	if(!(Root->restoreConfig() || Root->showConfigDialog()))
	{
		return false;
	}

	init();
	theGUI = GAME::GUI::getSingletonPtr();
	theGUI->initializeGUI();
	input = GAME::GameInput::getSingletonPtr();
	input->initialize(Window);

	Ogre::WindowEventUtilities::addWindowEventListener(Window, this);
	Root->addFrameListener(this);

	changeState(state);
	Root->startRendering();

	return true;
}

void Application::changeState(GAME::GameState* state)
{
	if( !m_States.empty() )
	{
		GAME::SoundManager::getSingletonPtr()->StopAllSounds();
		m_States.back()->Cleanup();
		m_States.pop_back();
	}

	m_States.push_back(state);
	m_States.back()->Init();
}

void Application::pushState(GAME::GameState* state)
{
	if(!m_States.empty())
		m_States.back()->Pause();

	m_States.push_back(state);
	m_States.back()->Init();
}

void Application::popState()
{
	if(!m_States.empty())
	{
		m_States.back()->Cleanup();
		m_States.pop_back();
	}

	if(!m_States.empty())
		m_States.back()->Resume();
}

GAME::GameState *Application::getGameState()
{
	return m_States.back();
}

GAME::GameInput *Application::getInput(void)
{
	return input;
}

GAME::GUI *Application::getGUI(void)
{
	return theGUI;
}

Ogre::Root *Application::getRoot(void)
{
	return Root;
}

void Application::setShutdown(bool shutdown)
{
	Shutdown = shutdown;
}

void Application::windowResized(Ogre::RenderWindow *rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width,height,depth,left,top);
	input->setWindowExtents(width,height);
}

void Application::windowClosed(Ogre::RenderWindow *rw)
{
	if (rw == Window)
	{
		if (input)
		{
			input->setWindowClosed();
		}
	}
}

bool Application::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	if(Window->isClosed())
		return false;

	if(Shutdown)
		return false;

	input->capture(evt.timeSinceLastFrame);

	bool isPlaying = false;
	soundMgr->GetSoundChannel(channel)->isPlaying(&isPlaying);

	if(!isPlaying)
	{
		if(soundMgr->GetSoundInstance(sound)->sound)
		{
			//sound = soundMgr->RemoveFromSoundInstanceVector(sound);
			soundMgr->GetSoundInstance(sound)->Clear();
			sound = INVALID_SOUND_INDEX;
		}

		result = playlist->getTag("FILE", count, &tag);
		if (result != FMOD_OK)
		{
			count = 0;
		}
		else
		{
			/*
				Get the first song in the playlist, create the sound and then play it.
			*/
			result = playlist->getTag("FILE", count, &tag);
			sound = soundMgr->CreateStream(std::string((char*)tag.data));
			soundMgr->PlaySound(sound,NULL,&channel);
			count++;

		}
	}

	return m_States.back()->frameRenderingQueued(evt);
}