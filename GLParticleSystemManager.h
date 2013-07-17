#ifndef __aPARTICLESYSTEMMANAGER_H__
#define __aPARTICLESYSTEMMANAGER_H__

#include <OgreRoot.h>
#include "GLParticleSystem.h"

namespace GAME
{
	class ParticleSystemManager : public Ogre::FrameListener
	{
	public:
		ParticleSystemManager();
		~ParticleSystemManager();

		void Init();
		void Cleanup();

		GAME::ParticleSystem *createParticleSystem(Ogre::Vector3 position, GAME::ParticleType type, std::string name = "");

	private:
		bool frameRenderingQueued(const Ogre::FrameEvent& evt);		

		std::vector<GAME::ParticleSystem> m_ParticleSystemList;
		int m_NumberOfParticles;
	};
}

#endif