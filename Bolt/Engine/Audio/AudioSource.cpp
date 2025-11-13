#include "../pch.hpp"
#include "../Audio/AudioSource.hpp"
#include "../Audio/AudioManager.hpp"


namespace Bolt {

	AudioSource::AudioSource(const AudioHandle& audioHandle)
		: m_audioHandle(audioHandle)
	{
	}

	void AudioSource::Play() {
		if (!m_audioHandle.IsValid()) {
			Logger::Error("AudioSource: Cannot play - invalid audio handle");
			return;
		}

		AudioManager::PlayAudioSource(*this);
	}

	void AudioSource::Pause() {
		if (m_instanceId == 0) {
			return;
		}

		AudioManager::PauseAudioSource(*this);

	}

	void AudioSource::Stop() {
		if (m_instanceId == 0) {
			return;
		}

		AudioManager::StopAudioSource(*this);
	}

	void AudioSource::Resume() {
		if (m_instanceId == 0) {
			return;
		}

		AudioManager::ResumeAudioSource(*this);
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
		if (!m_audioHandle.IsValid()) {
			Logger::Error("AudioSource: Cannot play one-shot - invalid audio handle");
			return;
		}

		AudioManager::PlayOneShot(m_audioHandle, m_Volume);
	}
}