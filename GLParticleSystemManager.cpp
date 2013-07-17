#include "GLParticleSystemManager.h"

namespace GAME
{
	ParticleSystemManager::ParticleSystemManager() { };

	ParticleSystemManager::~ParticleSystemManager() { };

	void ParticleSystemManager::Init()
	{
		Ogre::Root::getSingletonPtr()->addFrameListener(this);
	}

	void ParticleSystemManager::Cleanup()
	{
		while(!m_ParticleSystemList.empty())
		{
			m_ParticleSystemList.back().Cleanup();
			m_ParticleSystemList.pop_back();
		}

		m_ParticleSystemList.clear();

		Ogre::Root::getSingletonPtr()->removeFrameListener(this);
	}

	ParticleSystem *ParticleSystemManager::createParticleSystem(Ogre::Vector3 position, GAME::ParticleType type, std::string name)
	{
		GAME::ParticleSystem newParticleSystem;

		char temp[10];
		sprintf(temp, "%i", m_NumberOfParticles);
		name += temp;
		
		newParticleSystem.Init(position, type, name);

		m_ParticleSystemList.push_back(newParticleSystem);
		m_NumberOfParticles++;
		return &m_ParticleSystemList.back();
	}

	bool ParticleSystemManager::frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		GAME::ParticleSystem *particleSystem;
		for(unsigned int i = 0; i < m_ParticleSystemList.size(); i++)
		{
			particleSystem = &m_ParticleSystemList.at(i);
			particleSystem->m_TimeToLive -= evt.timeSinceLastFrame;
			if(particleSystem->m_TimeToLive <= 0)
			{
				particleSystem->Cleanup();
				m_ParticleSystemList.erase(m_ParticleSystemList.begin() + i);
			}
		}
		return true;
	}
}