#include "GLGUI.h"

#include <exception>

#include "GLapplication.h"
#include "GLGameState.h"

namespace GAME
{	
	GUI GUI::m_GUI;

	GUI::GUI(void) { }

	GUI::~GUI(void)	{ 
	// DESTROY ALL THE WINDOWS.
		CEGUI::WindowManager::getSingleton().destroyAllWindows();	
	}

	GUI *GUI::getSingletonPtr(void)
	{
		return &m_GUI;
	}

	bool GUI::initializeGUI(void)
	{
		mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

		CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
		CEGUI::Font::setDefaultResourceGroup("Fonts");
		CEGUI::Scheme::setDefaultResourceGroup("Schemes");
		CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
		CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

		CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
		CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");

		CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

		sheet = wmgr.createWindow("DefaultWindow", "ROOT");
		CEGUI::System::getSingleton().setGUISheet(sheet);

		CEGUI::ImagesetManager::getSingleton().create("Wireframe.imageset");
		//CEGUI::ImagesetManager::getSingleton().createFromImageFile("Wireframe", "vanilla.tga");
		
		return true;
	}

	void GUI::injectTimestamps(Ogre::Real timeSinceLastFrame)
	{
		CEGUI::System::getSingleton().injectTimePulse(timeSinceLastFrame);
	}

	void GUI::hideGUI(void)
	{
		sheet->setVisible(!sheet->isVisible());
	}

	CEGUI::Window *GUI::getSheet(void)
	{
		return sheet;
	}

	CEGUI::OgreRenderer *GUI::getRenderer(void)
	{
		return mRenderer;
	}

	/*void GUI::SetRenderQueue( Ogre::RenderQueueGroupID renderQueue, bool postQueue)
	{
		m_RenderQueue = renderQueue;
		m_PostQueue = postQueue;
	}

	void GUI::renderQueueStarted(Ogre::uint8 id, const Ogre::String& invocation, bool& skipThisQueue)
	{
		if( !m_PostQueue && m_RenderQueue == id && invocation == "" )
		{
			CEGUI::System::getSingleton().renderGUI();
		}
	}
	void GUI::renderQueueEnded(Ogre::uint8 id, const Ogre::String& invocation, bool& repeatThisQueue)
	{
		if( m_PostQueue && m_RenderQueue == id && invocation == "" )
		{
			CEGUI::System::getSingleton().renderGUI();
		}
	}*/
}