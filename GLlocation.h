#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreMovablePlane.h>

namespace GAME
{
	enum LocationType { BASE, RESOURCE, OBJECTIVE };

	struct Location
	{
		Location() : locationValue(3), controlledBy(0), oldControlledBy(0) { }
		Ogre::Vector3 mPosition;
		Ogre::Vector2 mDimension;
		Ogre::MovablePlane *mPlane;

		Ogre::Entity *mEntity;
		Ogre::SceneNode *mSceneNode;

		Ogre::SceneNode *m_BorderSceneNode;
		Ogre::ManualObject *m_BorderManualObject;

		LocationType type;
		//GAME::Commander *controlledBy;
		// Value of controlledBy mean which commander is controlling
		//  a location. ie. controlledBy = 2, commander2 controls that
		//  location.
		int controlledBy;
		int oldControlledBy;

		Ogre::Vector3 getPosition(void)
		{
			return mSceneNode->getPosition();
		}
		Ogre::Vector2 getDimension(void)
		{
			return mDimension;
		}

		int locationValue;
	};
}
#endif