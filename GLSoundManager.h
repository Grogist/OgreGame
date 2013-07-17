#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__

// Written using the FMOD SoundManager code snippet as a template
//  found at http://www.ogre3d.org/tikiwiki/FMOD+SoundManager

#include <fmod.hpp>
#include <fmod_errors.h>
#include <OgreSceneNode.h>
#include <OgreLogManager.h>

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

//#define MAX_SOUND_CHANNELS       100 //200
static const int MAX_SOUND_CHANNELS = 50;
 
#define INVALID_SOUND_INDEX      -1
#define INVALID_SOUND_CHANNEL    -1

namespace GAME
{
	typedef enum
	{
		SOUND_TYPE_INVALID = 0,
		SOUND_TYPE_3D_SOUND,
		SOUND_TYPE_3D_SOUND_LOOPED,
		SOUND_TYPE_2D_SOUND,
		SOUND_TYPE_2D_SOUND_LOOPED,
	} SOUND_TYPE;

	struct SoundInstance
	{
		SoundInstance()
		{
			filename.clear();
			sound = NULL;
			soundType = SOUND_TYPE_INVALID;
		};
		~SoundInstance()
		{
			//sound->release();
		};
		void Clear(void);
		std::string filename;
		FMOD::Sound *sound;
		SOUND_TYPE soundType;
	};

	// All Scene Nodes must be relative to Root;
	struct ChannelInstance
	{
		ChannelInstance()
		{
			sceneNode = NULL;
			prevPosition = Ogre::Vector3(0.0f,0.0f,0.0f);
		};
		~ChannelInstance() {};
		void Clear(void);
		Ogre::SceneNode *sceneNode;
		Ogre::Vector3 prevPosition;
	};

	class SoundManager
	{
	public:		
		~SoundManager();

		void Initialize(void);
		void StopAllSounds(void);
		// Listener Must has Root as it's Parent.
		void FrameStarted(Ogre::SceneNode *listener, Ogre::Real timeElapsed);

		int CreateSound(std::string &filename);
		int CreateStream(std::string &filename);
		int	CreateLoopedSound(std::string &filename);
		int CreateLoopedStream(std::string &filename);

		FMOD::Sound *CreatePlayList(std::string &filename);
		
		int CreateSound(std::string &filename, SOUND_TYPE soundtype);

		void PlaySound(int soundIndex, Ogre::SceneNode *soundNode, int *channelIndex);
		void StopSound(int *channelIndex);
		int  FindSound(std::string &filename, SOUND_TYPE soundtype); // returns sound index.

		void Set3DMinMaxDistance(int channelIndex, float minDistance, float maxDistance);

		// Returns in seconds.
		float GetSoundLength(int soundIndex);
		SoundInstance *GetSoundInstance(int soundIndex);
		//int RemoveFromSoundInstanceVector(int soundIndex);
		FMOD::Channel *GetSoundChannel(int channelIndex);

		void MuteSound(void);

		float getMasterVolume(void);
		void setMasterVolume(float volume);

		static SoundManager& getSingleton(void);
		static SoundManager* getSingletonPtr(void);

	private:
		SoundManager();

		FMOD::System	 *system;
		Ogre::Vector3	prevListenerPosition;
		
		std::vector<SoundInstance> soundInstanceVector;

		ChannelInstance	channelArray[MAX_SOUND_CHANNELS];

		void IncrementNextSoundInstanceIndex(void);

		static SoundManager *soundManager_Singleton;

		bool isMuted;

		float masterVolume;

		// Equivalent to an empty SoundInstance;
		SoundInstance empty;
	};
}
#endif