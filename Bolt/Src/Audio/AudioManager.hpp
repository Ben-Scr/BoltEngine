#pragma once

#include "../Audio/AudioHandle.hpp"

#include <miniaudio/miniaudio.h>
#include <queue>

namespace Bolt {
	class AudioSource;
	class Audio;

	class AudioManager {
	public:
		static constexpr uint32_t MAX_CONCURRENT_SOUNDS = 64;
		static constexpr uint32_t MAX_SOUNDS_PER_FRAME = 8;
		static constexpr float MIN_SOUND_INTERVAL = 0.1f;

		static bool Initialize();
		static void Shutdown();
		static void Update();


		static AudioHandle LoadAudio(const std::string& filepath);
		static void UnloadAudio(const AudioHandle& audioHandle);
		static void UnloadAllAudio();


		static void PlayAudioSource(AudioSource& source);
		static void PauseAudioSource(AudioSource& source);
		static void StopAudioSource(AudioSource& source);
		static void ResumeAudioSource(AudioSource& source);


		static void SetMasterVolume(float volume);
		static float GetMasterVolume() { return s_masterVolume; }


		static void PlayOneShot(const AudioHandle& audioHandle, float volume = 1.0f);

		static void SetMaxConcurrentSounds(uint32_t maxSounds);
		static void SetMaxSoundsPerFrame(uint32_t maxPerFrame);
		static uint32_t GetActiveSoundCount();


		static bool IsInitialized() { return s_IsInitialized; }
		static bool IsAudioLoaded(const AudioHandle& audioHandle);
		static const Audio* GetAudio(const AudioHandle& audioHandle);


		struct SoundInstance {
			ma_sound Sound;
			AudioHandle AudioHandle;
			bool IsValid = false;
		};

		static SoundInstance* GetSoundInstance(uint32_t instanceId);

	private:
		AudioManager() = delete;
		~AudioManager() = delete;
		AudioManager(const AudioManager&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;


		static ma_engine s_Engine;
		static bool s_IsInitialized;

		struct SoundRequest {
			AudioHandle GetHandle;
			float Volume;
			float Priority;
			glm::vec3 Position;
			bool Is3D;
			std::chrono::steady_clock::time_point RequestTime;


			bool operator<(const SoundRequest& other) const {
				return Priority < other.Priority;
			}
		};

		struct SoundLimitData {
			std::chrono::steady_clock::time_point LastPlayTime;
			uint32_t FramePlayCount;
		};


		static uint32_t s_maxConcurrentSounds;
		static uint32_t s_maxSoundsPerFrame;
		static uint32_t s_soundsPlayedThisFrame;
		static uint32_t s_activeSoundCount;


		static std::priority_queue<SoundRequest> s_soundQueue;
		static std::unordered_map<AudioHandle::HandleType, SoundLimitData> s_soundLimits;


		static bool CanPlaySound(const AudioHandle& blockTexture, float priority);
		static void ProcessSoundQueue();
		static void ThrottleSound(const AudioHandle& blockTexture);
		static bool IsThrottled(const AudioHandle& blockTexture);


		static std::unordered_map<AudioHandle::HandleType, std::unique_ptr<Audio>> s_audioMap;
		static AudioHandle::HandleType s_nextHandle;

		static std::vector<SoundInstance> s_soundInstances;
		static std::vector<uint32_t> s_freeInstanceIndices;


		static float s_masterVolume;


		static AudioHandle::HandleType GenerateHandle();
		static uint32_t CreateSoundInstance(const AudioHandle& audioHandle);
		static void DestroySoundInstance(uint32_t instanceId);
		static void CleanupFinishedSounds();

		static void UpdateListener();
		static void UpdateSoundInstances();

		friend class AudioSource;
	};
}