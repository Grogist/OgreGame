#ifndef __GUI_H__
#define __GUI_H__

#include <CEGUI.h>
#include <CEGUIOgreRenderer.h>

#include "GLlocation.h"
#include "GLActionList.h"

#include <OgrePrerequisites.h>
#include <OgreTimer.h>
#include <OgreRenderQueueListener.h>

namespace GAME
{
	class GUI // : public Ogre::RenderQueueListener
	{
	public:
		
		~GUI(void);

		bool initializeGUI(void);

		static GUI *getSingletonPtr(void);

		void injectTimestamps(Ogre::Real timeSinceLastFrame);

		void hideGUI(void);

		CEGUI::Window *getSheet(void);
		CEGUI::OgreRenderer *getRenderer(void);

		//void SetRenderQueue( Ogre::RenderQueueGroupID renderQueue, bool postQueue);
		//void renderQueueStarted(Ogre::uint8 id, const Ogre::String& invocation, bool& skipThisQueue);
		//void renderQueueEnded(Ogre::uint8 id, const Ogre::String& invocation, bool& repeatThisQueue);

	protected:
		CEGUI::OgreRenderer *mRenderer;

		CEGUI::Window *sheet;

	private:
		GUI();

		//Ogre::RenderQueueGroupID m_RenderQueue;
		//bool m_PostQueue;

		static GUI m_GUI;
	};	
}
#endif