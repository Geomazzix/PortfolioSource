#pragma once
#include "Core/ISystem.h"
#include <string>
#include <unordered_map>

#include <FMOD/studio/fmod_studio_common.h>

void ERRCHECK_fnAudio(FMOD_RESULT result, const char* file, int line);
#define ERRCHECKAUDIO(_result) ERRCHECK_fnAudio(_result, __FILE__, __LINE__)

namespace FMOD {
	class System;
	namespace Studio {
		class EventInstance;
		class EventDescription;
		class System;
		class Bank;
	}
}

namespace Frac
{
	class AudioSource;
	class AudioListener;
	class ECSWrapper;
	struct AudioSourceComponent;
	
	class AudioManager : public ISystem
	{
	public:
		AudioManager();
		~AudioManager() = default;
		
		/**
		 * \brief Should be called when closing the engine to make sure everything gets properly deleted.
		 */
		void ShutDown();
		/**
		 * \brief Update function for the audio manager should be called every frame to update the playback state of the audio
		 * \param deltaTime the time since last frame
		 */
		void Update(float deltaTime) override;
		/**
		 * \brief Load an audio bank from file.
		 * \param bankName the file location of the .bank file.
		 * \param flags how to load the bank into FMOD default should be fine for most use cases
		 */
		void LoadBank(const std::string& bankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags = FMOD_STUDIO_LOAD_BANK_NORMAL);
		
		/**
		 * \brief Add an audio event to a component
		 * \param audioSourceComponent The component you want the audio to be added at
		 * \param eventName The name of the event to add
		 */
		void AddEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName);

		/**
		 * \brief Checks whether the component has a specific event
		 * \param audioSourceComponent The component you want to check for the event
		 * \param eventName The name of the event to check
		 */
		bool HasEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName);

		/**
		 * \brief Play an audio event from a component
		 * \param audioSourceComponent The component you want the audio to be played from
		 * \param eventName The name of the event to play
		 * \param volume the volume of the sound 
		 */
		void PlayEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName);
		/**
		* 
		 * \brief Stop playing a certain event
		 * \param eventName the event to stop playing
		 */
		void StopEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName);

		/**
		 * \brief Sets the master volume for the entire engine
		 * \param volume the volume of all the sound events in the engine
		 */
		void SetMasterVolume(float volume);
		/**
		* \brief Gets the master volume for the entire engine
		* \return value of current master value
		*/
		float GetMasterValue() const;
		
		FMOD::Studio::System* GetStudioSystem();
		
	private:

		FMOD::Studio::EventInstance* GetEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName);
		FMOD::Studio::EventDescription* GetDescription(AudioSourceComponent& audioSourceComponent, const std::string& eventName);

		FMOD::System* m_system = nullptr;
		FMOD::Studio::System* m_studioSystem = nullptr;

		std::unordered_map<std::string, FMOD::Studio::Bank*> m_banks;	

		float m_masterVolume = 1.0f;
	};
} // namespace Frac
