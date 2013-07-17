/************************

GameCamera controls the functionality
of the camera

************************/

#ifndef __GameCamera_H__
#define __GameCamera_H__

#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

namespace GAME
{
	class GameCamera
	{
	public:
		GameCamera(void);
		~GameCamera(void);

		void initialize(Ogre::SceneManager *sceneMgr, Ogre::RenderWindow *window);

		Ogre::Camera *getCamera(void);
		Ogre::Viewport *getViewport(void);
		Ogre::SceneNode *getCameraSceneNode(void);

	private:	
		void createCamera(Ogre::SceneManager *sceneMgr);
		void createViewport(Ogre::RenderWindow *window);

		Ogre::Camera *mCamera;
		Ogre::Viewport *mVP;
		Ogre::SceneNode *cameraSceneNode;
	};
}
#endif