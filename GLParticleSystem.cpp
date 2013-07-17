#include "GLParticleSystem.h"

#include "GLPlayState.h"

namespace GAME
{
	ParticleSystem::ParticleSystem() : m_TimeToLive(0), m_SceneNode(NULL), m_ParticleSystem(NULL)
	{ };

	ParticleSystem::~ParticleSystem()
	{
		m_TimeToLive = 0;
		m_SceneNode = NULL;
		m_ParticleSystem = NULL;
	};

	void ParticleSystem::Init(Ogre::Vector3 position, GAME::ParticleType type, std::string name)
	{
		Ogre::SceneManager *sceneManager = GAME::PlayState::getSingletonPtr()->getSceneManager();
		m_SceneNode = sceneManager->getRootSceneNode()->createChildSceneNode(name, position);
		switch(type)
		{
		case FIREWALL:
			m_ParticleSystem = sceneManager->createParticleSystem(name, "1PlasmaShader");
			break;
		default:
			m_ParticleSystem = sceneManager->createParticleSystem(name, "Explosion");
			break;
		}
		
		m_SceneNode->attachObject(m_ParticleSystem);
		m_TimeToLive = m_ParticleSystem->getEmitter(0)->getDuration() +
			m_ParticleSystem->getEmitter(0)->getTimeToLive();
	}

	void ParticleSystem::Cleanup()
	{
		m_SceneNode->detachObject(m_ParticleSystem);
		Ogre::SceneManager *sceneManager = GAME::PlayState::getSingletonPtr()->getSceneManager();
		sceneManager->destroyParticleSystem(m_ParticleSystem);
		sceneManager->getRootSceneNode()->removeAndDestroyChild(m_SceneNode->getName());
	}
}