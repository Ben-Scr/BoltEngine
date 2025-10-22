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
		void SetSpatial(bool spatial);
		void SetPosition(const glm::vec3& position);
		void SetVelocity(const glm::vec3& velocity);
		void SetMinDistance(float distance);
		void SetMaxDistance(float distance);
		void SetAttenuationModel(int model);
		void SetAudioHandle(const AudioHandle& blockTexture);

	    void PlayOneShot();
		void FadeIn(float duration);
		void FadeOut(float duration);


		float GetVolume() const { return m_Volume; }
		float GetPitch() const { return m_Pitch; }
		bool IsLooping() const { return m_Loop; }
		bool IsSpatial() const { return m_Spatial; }
		bool IsPlaying() const;
		bool IsPaused() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		const AudioHandle& GetAudioHandle() const { return m_audioHandle; }


		const glm::vec3& GetVelocity() const { return m_Velocity; }
		float GetMinDistance() const { return m_MinDistance; }
		float GetMaxDistance() const { return m_MaxDistance; }
		int GetAttenuationModel() const { return m_AttenuationModel; }


		uint32_t GetInstanceId() const { return m_instanceId; }
		void SetInstanceId(uint32_t id) { m_instanceId = id; }

	private:
		AudioHandle m_audioHandle;
		uint32_t m_instanceId = 0;


		float m_Volume = 1.0f;
		float m_Pitch = 1.0f;
		bool m_Loop = false;
		bool m_Spatial = false;


		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_Velocity{ 0.0f };
		float m_MinDistance = 1.0f;
		float m_MaxDistance = 100.0f;
		int m_AttenuationModel = 0;
	};
}