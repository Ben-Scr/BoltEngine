#include "pch.hpp"
#include "Audio/AudioSource.hpp"
#include "Audio/AudioManager.hpp"


namespace Bolt {

	AudioSource::AudioSource(const AudioHandle& audioHandle)
		: m_audioHandle(audioHandle)
	{
	}

	void AudioSource::Play() {
		BOLT_ASSERT(m_audioHandle.IsValid(), BoltErrorCode::InvalidHandle, "Cannot play invalid audio handle");
		AudioManager::PlayAudioSource(*this);
	}

	void AudioSource::Pause() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot pause invalid audio handle");
		AudioManager::PauseAudioSource(*this);
	}

	void AudioSource::Stop() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot stop invalid audio handle");
		AudioManager::StopAudioSource(*this);
	}

	void AudioSource::Resume() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot resume invalid audio handle");
		AudioManager::ResumeAudioSource(*this);
	}

	void AudioSource::Destroy() {
		BOLT_ASSERT(m_instanceId != 0, BoltErrorCode::InvalidHandle, "Cannot destroy invalid audio handle");
		AudioManager::DestroySoundInstance(m_instanceId);
	}

	void AudioSource::SetVolume(float volume) {
		m_Volume = Max(0.0f, volume);

		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				float masterVolume = AudioManager::GetMasterVolume();
				ma_sound_set_volume(&instance->Sound, m_Volume * masterVolume);
			}
		}
	}

	void AudioSource::SetPitch(float pitch) {
		m_Pitch = Max(0.01f, pitch);

		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				ma_sound_set_pitch(&instance->Sound, m_Pitch);
			}
		}
	}

	void AudioSource::SetLoop(bool loop) {
		m_Loop = loop;


		if (m_instanceId != 0) {
			auto* instance = AudioManager::GetSoundInstance(m_instanceId);
			if (instance && instance->IsValid) {
				ma_sound_set_looping(&instance->Sound, m_Loop);
			}
		}
	}

	bool AudioSource::IsPlaying() const {
		if (m_instanceId == 0) {
			return false;
		}

		auto* instance = AudioManager::GetSoundInstance(m_instanceId);
		if (instance && instance->IsValid) {
			return ma_sound_is_playing(&instance->Sound);
		}

		return false;
	}

	bool AudioSource::IsPaused() const {
		if (m_instanceId == 0) {
			return false;
		}

		auto* instance = AudioManager::GetSoundInstance(m_instanceId);
		if (instance && instance->IsValid) {
			return !ma_sound_is_playing(&instance->Sound) && ma_sound_at_end(&instance->Sound) == MA_FALSE;
		}

		return false;
	}


	void AudioSource::SetAudioHandle(const AudioHandle& blockTexture) {
		if (IsPlaying()) {
			Stop();
		}

		m_audioHandle = blockTexture;
	}

	void AudioSource::PlayOneShot() {
		BOLT_ASSERT(m_audioHandle.IsValid(), BoltErrorCode::InvalidHandle, "AudioSource cannot play one-shot - invalid audio handle");
		AudioManager::PlayOneShot(m_audioHandle, m_Volume);
	}

	bool AudioSource::IsValid() const { return m_audioHandle.IsValid(); }
}