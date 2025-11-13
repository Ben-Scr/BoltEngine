#include "../pch.hpp"

#include "../Audio/AudioManager.hpp"
#include "../Audio/AudioSource.hpp"
#include "../Audio/Audio.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Bolt {
	ma_engine AudioManager::s_Engine{};
	bool AudioManager::s_IsInitialized = false;
	std::unordered_map<AudioHandle::HandleType, std::unique_ptr<Audio>> AudioManager::s_audioMap;
	AudioHandle::HandleType AudioManager::s_nextHandle = 1;
	std::vector<AudioManager::SoundInstance> AudioManager::s_soundInstances;
	std::vector<uint32_t> AudioManager::s_freeInstanceIndices;
	float AudioManager::s_masterVolume = 1.0f;

	uint32_t AudioManager::s_maxConcurrentSounds = MAX_CONCURRENT_SOUNDS;
	uint32_t AudioManager::s_maxSoundsPerFrame = MAX_SOUNDS_PER_FRAME;
	uint32_t AudioManager::s_soundsPlayedThisFrame = 0;
	uint32_t AudioManager::s_activeSoundCount = 0;
	std::priority_queue<AudioManager::SoundRequest> AudioManager::s_soundQueue;
	std::unordered_map<AudioHandle::HandleType, AudioManager::SoundLimitData> AudioManager::s_soundLimits;


	bool AudioManager::Initialize() {
		if (s_IsInitialized) {
			return true;
		}

		ma_result result = ma_engine_init(nullptr, &s_Engine);
		if (result != MA_SUCCESS) {
			std::cerr << "AudioManager: Failed to initialize miniaudio engine. Error: " << result << std::endl;
			return false;
		}

		s_soundInstances.reserve(256);
		s_freeInstanceIndices.reserve(256);

		UpdateListener();

		s_IsInitialized = true;
		return true;
	}

	void AudioManager::Shutdown() {
		if (!s_IsInitialized) {
			return;
		}

		for (auto& instance : s_soundInstances) {
			if (instance.IsValid) {
				ma_sound_stop(&instance.Sound);
				ma_sound_uninit(&instance.Sound);
			}
		}
		s_soundInstances.clear();
		s_freeInstanceIndices.clear();

		UnloadAllAudio();

		ma_engine_uninit(&s_Engine);
		s_IsInitialized = false;
	}

	void AudioManager::Update() {
		if (!s_IsInitialized) {
			return;
		}

		s_soundsPlayedThisFrame = 0;

		ProcessSoundQueue();
		CleanupFinishedSounds();
		UpdateListener();
		UpdateSoundInstances();


		s_activeSoundCount = 0;
		for (const auto& instance : s_soundInstances) {
			if (instance.IsValid && ma_sound_is_playing(&instance.Sound)) {
				s_activeSoundCount++;
			}
		}
	}

	bool AudioManager::CanPlaySound(const AudioHandle& blockTexture, float priority) {
		if (priority >= 2.0f) {
			return true;
		}


		if (s_activeSoundCount >= s_maxConcurrentSounds) {
			return priority > 1.5f;
		}

		if (s_soundsPlayedThisFrame >= s_maxSoundsPerFrame) {
			return priority > 1.8f;
		}

		return true;
	}

	void AudioManager::ProcessSoundQueue() {
		uint32_t processed = 0;
		const uint32_t maxProcessPerFrame = 4;

		while (!s_soundQueue.empty() && processed < maxProcessPerFrame &&
			s_soundsPlayedThisFrame < s_maxSoundsPerFrame &&
			s_activeSoundCount < s_maxConcurrentSounds) {

			SoundRequest request = s_soundQueue.top();
			s_soundQueue.pop();


			auto now = std::chrono::steady_clock::now();
			auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - request.RequestTime);
			if (age.count() > 200) {
				processed++;
				continue;
			}

			if (IsThrottled(request.Handle)) {
				processed++;
				continue;
			}


			if (request.Is3D) {
				Logger::Warning("There is no support for 3D Audio");
			}

			PlayOneShot(request.Handle, request.Volume);


			s_soundsPlayedThisFrame++;
			ThrottleSound(request.Handle);
			processed++;
		}
	}

	void AudioManager::ThrottleSound(const AudioHandle& blockTexture) {
		auto& limitData = s_soundLimits[blockTexture.GetHandle()];
		limitData.LastPlayTime = std::chrono::steady_clock::now();
		limitData.FramePlayCount++;
	}

	bool AudioManager::IsThrottled(const AudioHandle& blockTexture) {
		auto it = s_soundLimits.find(blockTexture.GetHandle());
		if (it == s_soundLimits.end()) {
			return false;
		}

		auto now = std::chrono::steady_clock::now();
		auto timeSinceLastPlay = std::chrono::duration_cast<std::chrono::milliseconds>(
			now - it->second.LastPlayTime);

		return timeSinceLastPlay.count() < (MIN_SOUND_INTERVAL * 1000);
	}

	void AudioManager::SetMaxConcurrentSounds(uint32_t maxSounds) {
		s_maxConcurrentSounds = Min(maxSounds, 128u);
	}

	void AudioManager::SetMaxSoundsPerFrame(uint32_t maxPerFrame) {
		s_maxSoundsPerFrame = Min(maxPerFrame, 16u);
	}

	uint32_t AudioManager::GetActiveSoundCount() {
		return s_activeSoundCount;
	}

	AudioHandle AudioManager::LoadAudio(const std::string& filepath) {
		if (!s_IsInitialized) {
			Logger::Error("AudioManager: Cannot load audio - AudioManager not initialized");
			return AudioHandle();
		}

		auto audio = std::make_unique<Audio>();

		if (!audio->LoadFromFile(filepath)) {
			Logger::Error("AudioManager: Failed to load audio from file: " + filepath);
			return AudioHandle();
		}

		AudioHandle::HandleType blockTexture = GenerateHandle();
		s_audioMap[blockTexture] = std::move(audio);
		return AudioHandle(blockTexture);
	}

	void AudioManager::UnloadAudio(const AudioHandle& blockTexture) {
		if (!blockTexture.IsValid()) {
			return;
		}

		auto it = s_audioMap.find(blockTexture.GetHandle());
		if (it != s_audioMap.end()) {

			for (auto& instance : s_soundInstances) {
				if (instance.IsValid && instance.AudioHandle == blockTexture) {
					ma_sound_stop(&instance.Sound);
					ma_sound_uninit(&instance.Sound);
					instance.IsValid = false;
				}
			}

			s_audioMap.erase(it);
		}
	}

	void AudioManager::UnloadAllAudio() {

		for (auto& instance : s_soundInstances) {
			if (instance.IsValid) {
				ma_sound_stop(&instance.Sound);
				ma_sound_uninit(&instance.Sound);
				instance.IsValid = false;
			}
		}

		s_audioMap.clear();
		s_nextHandle = 1;
	}

	void AudioManager::PlayAudioSource(AudioSource& source) {
		if (!s_IsInitialized || !source.GetAudioHandle().IsValid()) {
			return;
		}

		if (source.GetInstanceId() != 0) {
			StopAudioSource(source);
		}

		uint32_t instanceId = CreateSoundInstance(source.GetAudioHandle());
		if (instanceId == 0) {
			Logger::Error("AudioManager: Failed to create sound instance");
			return;
		}

		source.SetInstanceId(instanceId);
		SoundInstance* instance = GetSoundInstance(instanceId);

		if (instance) {
			ma_sound_set_volume(&instance->Sound, source.GetVolume() * s_masterVolume);
			ma_sound_set_pitch(&instance->Sound, source.GetPitch());
			ma_sound_set_looping(&instance->Sound, source.IsLooping());
			ma_sound_set_positioning(&instance->Sound, ma_positioning_relative);


			ma_result result = ma_sound_start(&instance->Sound);
			if (result != MA_SUCCESS) {
				Logger::Error("AudioManager: Failed to start sound playback. Error: " + result);
				DestroySoundInstance(instanceId);
				source.SetInstanceId(0);
			}
		}
	}

	void AudioManager::PauseAudioSource(AudioSource& source) {
		if (!s_IsInitialized || source.GetInstanceId() == 0) {
			return;
		}

		SoundInstance* instance = GetSoundInstance(source.GetInstanceId());
		if (instance && instance->IsValid) {
			ma_sound_stop(&instance->Sound);
		}
	}

	void AudioManager::StopAudioSource(AudioSource& source) {
		if (!s_IsInitialized || source.GetInstanceId() == 0) {
			return;
		}

		DestroySoundInstance(source.GetInstanceId());
		source.SetInstanceId(0);
	}

	void AudioManager::ResumeAudioSource(AudioSource& source) {
		if (!s_IsInitialized || source.GetInstanceId() == 0) {
			return;
		}

		SoundInstance* instance = GetSoundInstance(source.GetInstanceId());
		if (instance && instance->IsValid) {
			ma_sound_start(&instance->Sound);
		}
	}

	void AudioManager::SetMasterVolume(float volume) {
		s_masterVolume = Max(0.0f, volume);

		if (s_IsInitialized) {
			ma_engine_set_volume(&s_Engine, s_masterVolume);
		}
	}

	void AudioManager::PlayOneShot(const AudioHandle& audioHandle, float volume) {
		if (!s_IsInitialized || !audioHandle.IsValid()) {
			return;
		}

		const Audio* audio = GetAudio(audioHandle);
		if (!audio) {
			return;
		}


		ma_sound sound;
		ma_result result = ma_sound_init_from_file(&s_Engine, audio->GetFilepath().c_str(), 0, nullptr, nullptr, &sound);

		if (result == MA_SUCCESS) {
			ma_sound_set_volume(&sound, volume * s_masterVolume);
			ma_sound_start(&sound);

			// Note: The sound will automatically be cleaned up by miniaudio when it finishes
		}
		else {
			std::cerr << "AudioManager: Failed to create one-shot sound. Error: " << result << std::endl;
		}
	}

	bool AudioManager::IsAudioLoaded(const AudioHandle& audioHandle) {
		if (!audioHandle.IsValid()) {
			return false;
		}

		return s_audioMap.find(audioHandle.GetHandle()) != s_audioMap.end();
	}

	const Audio* AudioManager::GetAudio(const AudioHandle& blockTexture) {
		if (!blockTexture.IsValid()) {
			return nullptr;
		}

		auto it = s_audioMap.find(blockTexture.GetHandle());
		return (it != s_audioMap.end()) ? it->second.get() : nullptr;
	}

	AudioHandle::HandleType AudioManager::GenerateHandle() {
		return s_nextHandle++;
	}

	uint32_t AudioManager::CreateSoundInstance(const AudioHandle& audioHandle) {
		if (!audioHandle.IsValid()) {
			return 0;
		}

		const Audio* audio = GetAudio(audioHandle);
		if (!audio || !audio->IsLoaded()) {
			return 0;
		}

		uint32_t instanceId;

		if (!s_freeInstanceIndices.empty()) {
			instanceId = s_freeInstanceIndices.back();
			s_freeInstanceIndices.pop_back();
		}
		else {
			instanceId = static_cast<uint32_t>(s_soundInstances.size());
			s_soundInstances.emplace_back();
		}

		SoundInstance& instance = s_soundInstances[instanceId];

		ma_result result = ma_sound_init_from_file(&s_Engine, audio->GetFilepath().c_str(),
			0, nullptr, nullptr, &instance.Sound);

		if (result != MA_SUCCESS) {
			std::cerr << "AudioManager: Failed to create sound instance. Error: " << result << std::endl;
			s_freeInstanceIndices.push_back(instanceId);
			return 0;
		}

		instance.AudioHandle = audioHandle;
		instance.IsValid = true;

		return instanceId + 1;
	}

	void AudioManager::DestroySoundInstance(uint32_t instanceId) {
		if (instanceId == 0) {
			return;
		}

		uint32_t index = instanceId - 1;

		if (index >= s_soundInstances.size()) {
			return;
		}

		SoundInstance& instance = s_soundInstances[index];
		if (instance.IsValid) {
			ma_sound_stop(&instance.Sound);
			ma_sound_uninit(&instance.Sound);
			instance.IsValid = false;
			instance.AudioHandle = AudioHandle();

			s_freeInstanceIndices.push_back(index);
		}
	}

	AudioManager::SoundInstance* AudioManager::GetSoundInstance(uint32_t instanceId) {
		if (instanceId == 0) {
			return nullptr;
		}

		uint32_t index = instanceId - 1;

		if (index >= s_soundInstances.size()) {
			return nullptr;
		}

		SoundInstance& instance = s_soundInstances[index];
		return instance.IsValid ? &instance : nullptr;
	}

	void AudioManager::CleanupFinishedSounds() {
		for (size_t i = 0; i < s_soundInstances.size(); ++i) {
			SoundInstance& instance = s_soundInstances[i];

			if (instance.IsValid) {
				if (!ma_sound_is_playing(&instance.Sound)) {
					ma_sound_uninit(&instance.Sound);
					instance.IsValid = false;
					instance.AudioHandle = AudioHandle();
					s_freeInstanceIndices.push_back(static_cast<uint32_t>(i));
				}
			}
		}
	}

	void AudioManager::UpdateListener() {
		if (!s_IsInitialized) {
			return;
		}
	}

	void AudioManager::UpdateSoundInstances() {
		if (!s_IsInitialized) {
			return;
		}
	}

}