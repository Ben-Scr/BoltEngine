#include "pch.hpp"
#include "Components/AudioSourceComponent.hpp"
#include "Audio/AudioManager.hpp"


namespace Bolt {

	AudioSourceComponent::AudioSourceComponent(const AudioHandle& audioHandle)
		: m_audioHandle(audioHandle)
	{
	}

	void AudioSourceComponent::Play() {
		BOLT_ASSERT(m_audioHandle.IsValid(), BoltErrorCode::InvalidHandle, "Cannot play invalid audio handle");
		AudioManager::PlayAudioSource(*this);
	}

	void AudioSourceComponent::Pause() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot pause invalid audio handle");
		AudioManager::PauseAudioSource(*this);
	}

	void AudioSourceComponent::Stop() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot stop invalid audio handle");
		AudioManager::StopAudioSource(*this);
	}

	void AudioSourceComponent::Resume() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot resume invalid audio handle");
		AudioManager::ResumeAudioSource(*this);
	}

	void AudioSourceComponent::Destroy() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot destroy invalid audio handle");
		AudioManager::DestroySoundInstance(m_instanceId);
	}

	void AudioSourceComponent::SetVolume(float volume) {
		m_Volume = Max(0.0f, volume);

		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				float masterVolume = AudioManager::GetMasterVolume();
				ma_sound_set_volume(&instance->Sound, m_Volume * masterVolume);
			}
		}
	}

	void AudioSourceComponent::SetPitch(float pitch) {
		m_Pitch = Max(0.01f, pitch);

		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				ma_sound_set_pitch(&instance->Sound, m_Pitch);
			}
		}
	}

	void AudioSourceComponent::SetLoop(bool loop) {
		m_Loop = loop;


		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				ma_sound_set_looping(&instance->Sound, m_Loop);
			}
		}
	}

	bool AudioSourceComponent::IsPlaying() const {
		if (m_instanceId == 0) {
			return false;
		}

		auto* instance = AudioManager::GetSoundInstance(m_instanceId);
		if (instance && instance->IsValid) {
			return ma_sound_is_playing(&instance->Sound);
		}

		return false;
	}

	bool AudioSourceComponent::IsPaused() const {
		if (m_instanceId == 0) {
			return false;
		}

		auto* instance = AudioManager::GetSoundInstance(m_instanceId);
		if (instance && instance->IsValid) {
			return !ma_sound_is_playing(&instance->Sound) && ma_sound_at_end(&instance->Sound) == MA_FALSE;
		}

		return false;
	}


	void AudioSourceComponent::SetAudioHandle(const AudioHandle& blockTexture) {
		if (IsPlaying()) {
			Stop();
		}

		m_audioHandle = blockTexture;
	}

	void AudioSourceComponent::PlayOneShot() {
		BOLT_ASSERT(m_audioHandle.IsValid(), BoltErrorCode::InvalidHandle, "AudioSource cannot play one-shot - invalid audio handle");
		AudioManager::PlayOneShot(m_audioHandle, m_Volume);
	}

	bool AudioSourceComponent::IsValid() const { return m_audioHandle.IsValid(); }
}