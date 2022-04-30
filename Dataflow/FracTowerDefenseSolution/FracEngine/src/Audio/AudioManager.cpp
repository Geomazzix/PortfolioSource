#include <pch.h>
#include <Audio/AudioManager.h>
#include <Audio/AudioComponent.h>

#include <FMOD/core/fmod_errors.h>
#include <FMOD/studio/fmod_studio.hpp>

void ERRCHECK_fnAudio(FMOD_RESULT result, const char* file, int line)
{
	if (result != FMOD_OK)
	{
		#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		LOGINFO("%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString(result));
		#endif
	}
}

namespace Frac
{
	AudioManager::AudioManager() : ISystem(eStage::OnUpdate)
	{
		ERRCHECKAUDIO(FMOD::Studio::System::create(&m_studioSystem));
		ERRCHECKAUDIO(m_studioSystem->getCoreSystem(&m_system));
		ERRCHECKAUDIO(m_system->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));

#if defined (PLATFORM_SWITCH)
		ERRCHECKAUDIO(m_studioSystem->initialize(128, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, nullptr));
#else
		ERRCHECKAUDIO(m_studioSystem->initialize(128, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, nullptr));
#endif

		LOGINFO("Fmod Initialization Complete");
	}

	void AudioManager::ShutDown()
	{
		ERRCHECKAUDIO(m_studioSystem->unloadAll());
		ERRCHECKAUDIO(m_studioSystem->flushCommands());
		ERRCHECKAUDIO(m_studioSystem->release());
	}

	void AudioManager::Update(float dt)
	{
		ERRCHECKAUDIO(m_studioSystem->update());
	}

	void AudioManager::LoadBank(const std::string& bankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags)
	{
		auto foundIt = m_banks.find(FileIO::GetPathFromWildcard(bankName));
		if (foundIt != m_banks.end())
			return;

		FMOD::Studio::Bank* bank;
		ERRCHECKAUDIO(m_studioSystem->loadBankFile(FileIO::GetPathFromWildcard(bankName).c_str(), flags, &bank));
		if (bank)
			m_banks[bankName] = bank;
	}

	void AudioManager::AddEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		FMOD::Studio::EventInstance* audioEvent = GetEvent(audioSourceComponent, eventName);
		FMOD::Studio::EventDescription* audioDescription = GetDescription(audioSourceComponent, eventName);
		if (audioEvent == nullptr && audioDescription == nullptr) {
			ERRCHECKAUDIO(m_studioSystem->getEvent(eventName.c_str(), &audioDescription));
			ERRCHECKAUDIO(audioDescription->createInstance(&audioEvent));
			audioSourceComponent.descriptions[eventName] = audioDescription;
			audioSourceComponent.events[eventName] = audioEvent;
		}
		else {
			LOGWARNING("Audio event already exists!");
		}
	}

	bool AudioManager::HasEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		FMOD::Studio::EventInstance* audioEvent = GetEvent(audioSourceComponent, eventName);
		if (audioEvent) {
			return true;
		}
		else {
			return false;
		}
	}

	void AudioManager::PlayEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		FMOD::Studio::EventInstance* audioEvent = GetEvent(audioSourceComponent, eventName);
		audioEvent->setVolume(m_masterVolume);
		if (audioEvent) {
			ERRCHECKAUDIO(audioEvent->start());
		}
		else {
			LOGWARNING("Audio event doesn't exist!");
		}
	}

	void AudioManager::StopEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		FMOD::Studio::EventInstance* audioEvent = GetEvent(audioSourceComponent, eventName);
		if (audioEvent) {
			ERRCHECKAUDIO(audioEvent->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
		}
		else {
			LOGWARNING("Audio event doesn't exist!");
		}
	}

	void AudioManager::SetMasterVolume(float volume)
	{
		m_masterVolume = volume;
		EngineCore::GetInstance().GetEnttRegistry().view<AudioSourceComponent>().each([this, volume](const auto entity, AudioSourceComponent& audio)
		{
			for (auto& it: audio.events)
			{
				it.second->setVolume(volume);
			}
		});
	}

	float AudioManager::GetMasterValue() const
	{
		return m_masterVolume;
	}

	FMOD::Studio::System* AudioManager::GetStudioSystem()
	{
		return m_studioSystem;
	}
	FMOD::Studio::EventInstance* AudioManager::GetEvent(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		if (!audioSourceComponent.events.empty()) {
			auto found = audioSourceComponent.events.find(eventName);
			if (found != audioSourceComponent.events.end()) {
				return found->second;
			}
		}

		return nullptr;
	}
	FMOD::Studio::EventDescription* AudioManager::GetDescription(AudioSourceComponent& audioSourceComponent, const std::string& eventName)
	{
		if (!audioSourceComponent.descriptions.empty()) {
			auto found = audioSourceComponent.descriptions.find(eventName);
			if (found != audioSourceComponent.descriptions.end()) {
				return found->second;
			}
		}

		return nullptr;
	}
} // namespace Frac


