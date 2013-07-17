#include "GLcamera.h"

namespace GAME
{

	GameCamera::GameCamera(void) {	}

	GameCamera::~GameCamera(void) {	}

	void GameCamera::initialize(Ogre::SceneManager *sceneMgr, Ogre::RenderWindow *window)
	{
		createCamera(sceneMgr);

		createViewport(window);
	}

	Ogre::Camera *GameCamera::getCamera(void)
	{
		return mCamera;
	}

	Ogre::Viewport *GameCamera::getViewport(void)
	{
		return mVP;
	}

	Ogre::SceneNode *GameCamera::getCameraSceneNode(void)
	{
		return cameraSceneNode;
	}

	void GameCamera::createCamera(Ogre::SceneManager *sceneMgr)
	{
		mCamera = sceneMgr->createCamera("PlayerCam");
		cameraSceneNode = sceneMgr->createSceneNode("PlayerCamSceneNode");
		cameraSceneNode->attachObject(mCamera);

		// MAKE THIS BETTER!!!
		Ogre::Real height = 800.0f ;
		Ogre::Real angle = 45.0f;
		Ogre::Real hypotenuse = height / Ogre::Math::Cos(angle);

		Ogre::Real x = hypotenuse * Ogre::Math::Sin(angle);
		Ogre::Real y = hypotenuse * Ogre::Math::Cos(angle);

		mCamera->setPosition(Ogre::Vector3(0,height,0));
		mCamera->lookAt(Ogre::Vector3(x,0,0));

		mCamera->setNearClipDistance(1);
	}

	void GameCamera::createViewport(Ogre::RenderWindow *window)
	{
		mVP = window->addViewport(mCamera);
		mVP->setBackgroundColour(Ogre::ColourValue(0,0,0));
		mVP->setOverlaysEnabled(false);
	
		mCamera->setAspectRatio(Ogre::Real(mVP->getActualWidth()) / Ogre::Real(mVP->getActualHeight()));
	}
}