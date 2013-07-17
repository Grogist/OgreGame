#ifndef __FIREWALL_H__
#define __FIREWALL_H__

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreMovablePlane.h>
#include <OgreTimer.h>

namespace GAME
{
	struct Firewall
	{
		Firewall() : damage(12), damageRate(300.0f) { damageTimer.reset(); }
		Ogre::Vector3 mPosition;
		Ogre::Vector2 mDimension;
		Ogre::MovablePlane *mPlane;

		Ogre::Entity *mEntity;
		Ogre::SceneNode *mSceneNode;

		Ogre::Vector3 getPosition(void)
		{
			return mSceneNode->getPosition();
		}

		float damageRate;

		Ogre::Timer damageTimer;

		int damage;
	};
}
#endif