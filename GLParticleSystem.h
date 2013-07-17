#ifndef __aPARTICLESYSTEM_H__
#define __aPARTICLESYSTEM_H__

#include <OgreVector3.h>

namespace GAME
{
	enum ParticleType
	{
		FIREBALL,
		FIREWALL
	};

	class ParticleSystem
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		void Init(Ogre::Vector3 position, GAME::ParticleType type, std::string name);
		void Cleanup();

		float m_TimeToLive;
		Ogre::SceneNode *m_SceneNode;
		Ogre::ParticleSystem *m_ParticleSystem;
	};
}

#endif