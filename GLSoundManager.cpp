#include "GLSoundManager.h"

#define INITIAL_VECTOR_SIZE 100
#define INCREASE_VECTOR_SIZE 20

#define DOPPLER_SCALE 1.0
#define DISTANCE_FACTOR 1.0
#define ROLLOFF_SCALE 0.5

// HARDCODED FOR NOW.
//#define SOUND_DIRECTORY "../Sound/"
static const char *SOUND_DIRECTORY = "../Sound/";

namespace GAME
{
	//template<> SoundManager* Singleton<SoundManager>::ms_Singleton = 0;
	SoundManager *SoundManager::soundManager_Singleton;

	void SoundInstance::Clear(void)
	{
		filename.clear();
		sound->release();
		sound = NULL;
		soundType = SOUND_TYPE_INVALID;
	}

	void ChannelInstance::Clear(void)
	{
		sceneNode = NULL;
		prevPosition = Ogre::Vector3(0.0f,0.0f,0.0f);
	}

	SoundManager::SoundManager() : isMuted(false), masterVolume(0.5f)
	{
		system = NULL;
		prevListenerPosition = Ogre::Vector3(0.0f,0.0f,0.0f);
	}

	SoundManager::~SoundManager()
	{
		if(system)
		{
			system->release();
			system->close();
		}
	}

	void SoundManager::Initialize(void)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage("*** Star of SoundManager::Initialize ***");
		FMOD_RESULT result;

		result = FMOD::System_Create(&system);
		if(result != FMOD_OK)
		{
			//Ogre::LogManager::getSingleton().logMessage("Error!");
			//Ogre::Exception::ERR_INTERNAL_ERROR, "FMOD error!", FMOD_ErrorString(result), "SoundManager::Initialize";
		}
		result = system->init(MAX_SOUND_CHANNELS, FMOD_INIT_NORMAL, 0);
		if(result != FMOD_OK)
		{

		}

		system->set3DSettings(DOPPLER_SCALE, DISTANCE_FACTOR, ROLLOFF_SCALE);

		Ogre::LogManager::getSingleton().logMessage("*** SoundManager Initialized. ***");
	}

	SoundManager *SoundManager::getSingletonPtr(void)
	{
		if(!soundManager_Singleton)
		{
			soundManager_Singleton = new SoundManager;
		}
		return soundManager_Singleton;
	}

	SoundManager &SoundManager::getSingleton(void)
	{
		assert(soundManager_Singleton);
		return *soundManager_Singleton;
	}

	void SoundManager::FrameStarted(Ogre::SceneNode *listenerNode, Ogre::Real timeElapsed)
	{
		int channelIndex;
		FMOD::Channel *nextChannel;
		FMOD_VECTOR	listenerPosition;
		FMOD_VECTOR	listenerForward;
		FMOD_VECTOR	listenerUp;
		FMOD_VECTOR	listenerVelocity;
		Ogre::Vector3 vectorVelocity;
		Ogre::Vector3 vectorForward;
		Ogre::Vector3 vectorUp;

		if(timeElapsed>0)
			vectorVelocity = (listenerNode->getPosition() - prevListenerPosition) / timeElapsed;
		else
			vectorVelocity = Ogre::Vector3(0.0f,0.0f,0.0f);

		vectorForward = listenerNode->getOrientation().zAxis();
		vectorForward.normalise();

		vectorUp = listenerNode->getOrientation().yAxis();
		vectorUp.normalise();

		listenerPosition.x = listenerNode->getPosition().x;
		listenerPosition.y = listenerNode->getPosition().y;
		listenerPosition.z = listenerNode->getPosition().z;

		listenerForward.x = vectorForward.x;
		listenerForward.y = vectorForward.y;
		listenerForward.z = vectorForward.z;

		listenerUp.x = vectorUp.x;
		listenerUp.y = vectorUp.y;
		listenerUp.z = vectorUp.z;

		listenerVelocity.x = vectorVelocity.x;
		listenerVelocity.y = vectorVelocity.y;
		listenerVelocity.z = vectorVelocity.z;

		system->set3DListenerAttributes(0, &listenerPosition, &listenerVelocity, &listenerForward, &listenerUp);
		system->update();

		prevListenerPosition = listenerNode->getPosition();

		for(channelIndex = 0; channelIndex < MAX_SOUND_CHANNELS; channelIndex++)
		{
			if(channelArray[channelIndex].sceneNode != NULL)
			{
				system->getChannel(channelIndex, &nextChannel);
				if(timeElapsed > 0)
						vectorVelocity = (channelArray[channelIndex].sceneNode->getPosition()
							- channelArray[channelIndex].prevPosition) / timeElapsed;
				else
					vectorVelocity = (Ogre::Vector3(0,0,0));

				listenerPosition.x = channelArray[channelIndex].sceneNode->getPosition().x;
				listenerPosition.y = channelArray[channelIndex].sceneNode->getPosition().y;
				listenerPosition.z = channelArray[channelIndex].sceneNode->getPosition().z;

				listenerVelocity.x = vectorVelocity.x;
				listenerVelocity.y = vectorVelocity.y;
				listenerVelocity.z = vectorVelocity.z;

				nextChannel->set3DAttributes(&listenerPosition, &listenerVelocity);
				channelArray[channelIndex].prevPosition = channelArray[channelIndex].sceneNode->getPosition();
			}
		}
	}

	int SoundManager::CreateStream(std::string &filename)
	{
		return CreateSound(filename, SOUND_TYPE_2D_SOUND);
	}

	int SoundManager::CreateSound(std::string &fileName)
	{
		return CreateSound(fileName, SOUND_TYPE_3D_SOUND);
	}
 
	int SoundManager::CreateLoopedSound(std::string &fileName)
	{
		return CreateSound(fileName, SOUND_TYPE_3D_SOUND_LOOPED);
	}
 
	int SoundManager::CreateLoopedStream(std::string &fileName)
	{
		return CreateSound(fileName, SOUND_TYPE_2D_SOUND_LOOPED);
	}

	FMOD::Sound *SoundManager::CreatePlayList(std::string &fileName)
	{
		FMOD_RESULT result;
		FMOD::Sound *playlist;
		std::string fullPathName;
		bool isPlayList = false;
		FMOD_SOUND_TYPE soundType;

		fullPathName = SOUND_DIRECTORY + fileName;
		
		result = system->createSound(fullPathName.c_str(), FMOD_DEFAULT, 0, &playlist);

		if(result != FMOD_OK)
		{
			Ogre::LogManager::getSingleton().logMessage(
				"SoundManager::CreateSound could not load sound '" + fileName + "' (invalid soundType)");
			return NULL;
		}

		result = playlist->getFormat(&soundType,0,0,0);

		isPlayList = (soundType == FMOD_SOUND_TYPE_PLAYLIST);

		if(isPlayList)
		{
			return playlist;
		}
		else
		{
			playlist->release();
			return NULL;
		}
	}

	int SoundManager::CreateSound(std::string &fileName, SOUND_TYPE soundType)
	{
		FMOD_RESULT result;
		FMOD::Sound *sound;
		SoundInstance newSoundInstance;
		std::string fullPathName;

		//fullPathName = SOUND_DIRECTORY + fileName;
		fullPathName = fileName;

		//IncrementNextSoundInstanceIndex();

		newSoundInstance.filename = fullPathName;
		newSoundInstance.soundType = soundType;
		
		if(boost::filesystem::exists(fullPathName))
		{
			switch(soundType)
			{
				case SOUND_TYPE_3D_SOUND:
				{
					result = system->createSound(fullPathName.c_str(), FMOD_3D | FMOD_HARDWARE, 0, &sound);
					break;
				}
				case SOUND_TYPE_3D_SOUND_LOOPED:
				{
					result = system->createSound(fullPathName.c_str(), FMOD_LOOP_NORMAL | FMOD_3D | FMOD_HARDWARE, 0, &sound);
					break;
				}
				case SOUND_TYPE_2D_SOUND:
				{
					result = system->createStream(fullPathName.c_str(), FMOD_DEFAULT, 0, &sound);
					break;
				}
				case SOUND_TYPE_2D_SOUND_LOOPED:
				{
					result = system->createStream(fullPathName.c_str(), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_HARDWARE, 0, &sound);
				}
				default:
				{
					Ogre::LogManager::getSingleton().logMessage(
						"SoundManager::CreateSound could not load sound '" + fileName + "' (invalid soundType)");
					return INVALID_SOUND_INDEX;
				}
			}

			if(result != FMOD_OK)
			{
				Ogre::LogManager::getSingleton().logMessage(
					"SoundManager::CreateSound could not load sound '" + fileName + "' (invalid soundType)");
				return INVALID_SOUND_INDEX;
			}

			newSoundInstance.sound = sound;

			soundInstanceVector.push_back(newSoundInstance);
		}

		return soundInstanceVector.size()-1;
	}

	void SoundManager::PlaySound(int soundIndex, Ogre::SceneNode *soundNode, int *channelIndex)
	{
		int			  channelIndexTemp;
		FMOD_RESULT	  result;
		FMOD_VECTOR	  initialPosition;
		FMOD::Channel *channel;
		SoundInstance soundInstance;

		if(soundIndex <= INVALID_SOUND_INDEX)
			return;

		if(channelIndex)
			channelIndexTemp = *channelIndex;
		else
			channelIndexTemp = INVALID_SOUND_CHANNEL;

		assert((soundIndex >= 0) && (soundIndex <= (int)soundInstanceVector.capacity()));

		if(channelIndexTemp != INVALID_SOUND_CHANNEL)
		{
			if(channelArray[channelIndexTemp].sceneNode != NULL)
			{
			result = system->getChannel(channelIndexTemp, &channel);
			if(result == FMOD_OK)
			{
				bool isPlaying;

				result = channel->isPlaying(&isPlaying);
				if((result == FMOD_OK) && (isPlaying==true) && channelArray[channelIndexTemp].sceneNode == soundNode)
				{
					return;
				}
			}
			}
		}

		soundInstance = soundInstanceVector.at(soundIndex);

		system->createSound(soundInstance.filename.c_str(), FMOD_HARDWARE, 0, &soundInstance.sound);
		
		result = system->playSound(FMOD_CHANNEL_FREE, soundInstance.sound, true, &channel);

		if(result != FMOD_OK)
		{
			
			Ogre::LogManager::getSingleton().logMessage(
				std::string("SoundManager::PlaySound could not play sound  FMOD Error:") + FMOD_ErrorString(result));
			if(channelIndex)
				*channelIndex = INVALID_SOUND_CHANNEL;
			return;
		}
		
		channel->getIndex(&channelIndexTemp);
		channelArray[channelIndexTemp].sceneNode = soundNode;

		if(soundNode)
		{
			Ogre::Vector3 worldPosition = soundNode->convertLocalToWorldPosition(Ogre::Vector3(0,0,0));
			channelArray[channelIndexTemp].prevPosition = worldPosition;

			initialPosition.x = worldPosition.x;
			initialPosition.y = worldPosition.y;
			initialPosition.z = worldPosition.z;
			channel->set3DAttributes(&initialPosition, NULL);
		}

		result = channel->setVolume(1.0);

		result = channel->setPaused(false);
		
		result = channel->setMute(isMuted);

		result = channel->setVolume(masterVolume);

		if(channelIndex)
			*channelIndex = channelIndexTemp;			
	}

	SoundInstance *SoundManager::GetSoundInstance(int soundIndex)
	{
		if( soundIndex > INVALID_SOUND_INDEX && (unsigned)soundIndex < soundInstanceVector.size() )
			return &soundInstanceVector.at(soundIndex);
		else
		{
			return &empty;
		}
	}

	/*int SoundManager::RemoveFromSoundInstanceVector(int soundIndex)
	{
		soundInstanceVector.at(soundIndex).Clear();
		soundInstanceVector.erase(soundInstanceVector.begin()+soundIndex);
		return INVALID_SOUND_INDEX;
	}*/

	FMOD::Channel *SoundManager::GetSoundChannel(int channelIndex)
	{
		if(channelIndex == INVALID_SOUND_CHANNEL)
			return NULL;

		FMOD::Channel *soundChannel;

		assert((channelIndex > 0) && (channelIndex < MAX_SOUND_CHANNELS));

		system->getChannel(channelIndex, &soundChannel);
		return soundChannel;
	}

	void SoundManager::Set3DMinMaxDistance(int channelIndex, float minDistance, float maxDistance)
	{
		FMOD_RESULT result;
		FMOD::Channel *channel;

		if(channelIndex == INVALID_SOUND_CHANNEL)
			return;

		result = system->getChannel(channelIndex, &channel);
		if(result == FMOD_OK)
			channel->set3DMinMaxDistance(minDistance, maxDistance);
	}

	void SoundManager::StopAllSounds(void)
	{
		int channelIndex;
		FMOD_RESULT result;
		FMOD::Channel *nextChannel;

		for (channelIndex = 0; channelIndex< MAX_SOUND_CHANNELS; channelIndex++)
		{
			result = system->getChannel(channelIndex, &nextChannel);
			if( result==FMOD_OK && nextChannel != NULL )
				nextChannel->stop();
			channelArray[channelIndex].Clear();
		}
	}

	void SoundManager::StopSound(int *channelIndex)
	{
		if(*channelIndex == INVALID_SOUND_CHANNEL)
			return;

		FMOD::Channel *soundChannel;

		assert((*channelIndex>0) && (*channelIndex < MAX_SOUND_CHANNELS));

		system->getChannel(*channelIndex, &soundChannel);

		soundChannel->stop();

		channelArray[*channelIndex].Clear();
		*channelIndex = INVALID_SOUND_CHANNEL;
	}

	int SoundManager::FindSound(std::string &fileName, SOUND_TYPE soundType)
	{
		int vectorIndex;
		int vectorCapacity;
		SoundInstance nextSoundInstance;

		vectorCapacity = (int)soundInstanceVector.capacity();
		for(vectorIndex = 0; vectorIndex < vectorCapacity; vectorIndex++)
		{
			nextSoundInstance = soundInstanceVector.at(vectorIndex);
			if((soundType == nextSoundInstance.soundType) && (fileName == nextSoundInstance.filename))
				return vectorIndex;
		}

		return INVALID_SOUND_INDEX;
	}

	float SoundManager::GetSoundLength(int soundIndex)
	{
		if (soundIndex <= INVALID_SOUND_INDEX)
			return 0.0f;

		assert((soundIndex >= 0) && (soundIndex < (int)soundInstanceVector.capacity()));

		unsigned int soundLength; //length in milliseconds;
		FMOD_RESULT result;
		SoundInstance *soundInstance;

		soundInstance = &soundInstanceVector.at(soundIndex);

		if (soundInstance)
		{
			result = soundInstance->sound->getLength(&soundLength, FMOD_TIMEUNIT_MS);
			if (result != FMOD_OK)
			{
				Ogre::LogManager::getSingleton().logMessage("SoundManager::GetSoundLength could not get length  FMOD Error");
				return 0.0f;
			}
		}
		else
		{
			Ogre::LogManager::getSingleton().logMessage("SoundManager::GetSoundLength could not find soundInstance");
			return 0.0f;
		}

		return (float)soundLength / 1000.0f;
	}

	void SoundManager::MuteSound(void)
	{
		int channelIndex;
		FMOD_RESULT result;
		FMOD::Channel *nextChannel;

		isMuted = !isMuted;

		for (channelIndex = 0; channelIndex< MAX_SOUND_CHANNELS; channelIndex++)
		{
			result = system->getChannel(channelIndex, &nextChannel);
			if( result==FMOD_OK && nextChannel != NULL )
			{
				nextChannel->setMute(isMuted);
			}
			channelArray[channelIndex].Clear();
		}
	
	}

	float SoundManager::getMasterVolume(void)
	{
		return masterVolume;
	}

	void SoundManager::setMasterVolume(float volume)
	{
		int channelIndex;
		FMOD_RESULT result;
		FMOD::Channel *nextChannel;

		masterVolume = volume;

		for (channelIndex = 0; channelIndex< MAX_SOUND_CHANNELS; channelIndex++)
		{
			result = system->getChannel(channelIndex, &nextChannel);
			if( result==FMOD_OK && nextChannel != NULL )
			{
				nextChannel->setVolume(masterVolume);
			}
			channelArray[channelIndex].Clear();
		}
	}
}