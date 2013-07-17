#ifndef __MINIMAP_H__
#define __MINIMAP_H__

#include "GLlocation.h"
#include "GLunit.h"

#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>

#include <OgreHardwarePixelBuffer.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>

namespace GAME
{
	class Minimap
	{
	public:
		Minimap () {};
		~Minimap () {};

		void Init(Ogre::SceneManager *sceneMgr, Ogre::Root *root, std::string mapName );
		void update(void);
		void Cleanup(void);

		Ogre::TexturePtr getTexture(void);
		Ogre::Camera *getCamera(void);
		void setCamera(Ogre::Vector2 worldsize);
		Ogre::Viewport *getViewport(void);

		Ogre::Overlay *overlay;

		// Overlay, controllerNumber
		std::vector<std::pair<Ogre::Overlay*, int*>> m_LocationOverlayList;
		std::vector<std::pair<Ogre::Overlay*, GAME::UnitController*>> m_UnitOverlayList;

		void makeLocationOverlays(std::deque<GAME::Location> *locationList);
		void makeUnitOverlay(GAME::UnitController *unit);
		void removeUnitOverlay(GAME::UnitController *unit);

	private:
		Ogre::Camera *renderCamera;
		Ogre::Viewport *v;
		Ogre::TexturePtr tex;
		Ogre::RenderTexture *rtex;

		
		int numberOfOverLays;
	};
}
#endif