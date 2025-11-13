#pragma once
#include "../Audio/AudioHandle.hpp"
#include <glm/vec3.hpp>

namespace Bolt {

	class AudioSource {
	public:
		AudioSource() = default;
		AudioSource(const AudioHandle& audioHandle);


		void Play();
		void Pause();
		void Stop();
		void Resume();


		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetLoop(bool loop);
		void SetAudioHandle(const AudioHandle& audioHandle);

		void PlayOneShot();


		float GetVolume() const { return m_Volume; }
		float GetPitch() const { return m_Pitch; }
		bool IsLooping() const { return m_Loop; }
		bool IsPlaying() const;
		bool IsPaused() const;

		const AudioHandle& GetAudioHandle() const { return m_audioHandle; }


		uint32_t GetInstanceId() const { return m_instanceId; }
		void SetInstanceId(uint32_t id) { m_instanceId = id; }

	private:
		AudioHandle m_audioHandle;
		uint32_t m_instanceId = 0;


		float m_Volume = 1.0f;
		float m_Pitch = 1.0f;
		bool m_Loop = false;
	};
}