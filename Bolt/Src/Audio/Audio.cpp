#include "pch.hpp"
#include "Audio/Audio.hpp"
#include <cstring>
#include <iostream>

namespace Bolt {

	Audio::~Audio() {
		Cleanup();
	}

	Audio::Audio(Audio&& other) noexcept
		: m_Decoder(other.m_Decoder)
		, m_IsLoaded(other.m_IsLoaded)
		, m_Filepath(std::move(other.m_Filepath))
	{
		std::memset(&other.m_Decoder, 0, sizeof(ma_decoder));
		other.m_IsLoaded = false;
		other.m_Filepath.clear();
	}

	Audio& Audio::operator=(Audio&& other) noexcept {
		if (this != &other) {
			Cleanup();

			m_Decoder = other.m_Decoder;
			m_IsLoaded = other.m_IsLoaded;
			m_Filepath = std::move(other.m_Filepath);

			std::memset(&other.m_Decoder, 0, sizeof(ma_decoder));
			other.m_IsLoaded = false;
			other.m_Filepath.clear();
		}
		return *this;
	}

	bool Audio::LoadFromFile(const std::string& filepath) {
		BOLT_ASSERT(!filepath.empty(), BoltErrorCode::LoadFailed, "Audio: Cannot load - filepath is empty");

		Cleanup();


		ma_decoder_config config = ma_decoder_config_init_default();
		ma_result result = ma_decoder_init_file(filepath.c_str(), &config, &m_Decoder);

		BOLT_ASSERT(result == MA_SUCCESS, BoltErrorCode::LoadFailed, "Audio: Failed to load audio file: " + filepath);

		m_Filepath = filepath;
		m_IsLoaded = true;

		return true;
	}

	uint32_t Audio::GetSampleRate() const {
		if (!m_IsLoaded) {
			return 0;
		}

		return m_Decoder.outputSampleRate;
	}

	uint32_t Audio::GetChannels() const {
		if (!m_IsLoaded) {
			return 0;
		}

		return m_Decoder.outputChannels;
	}

	uint64_t Audio::GetFrameCount() const {
		if (!m_IsLoaded) {
			return 0;
		}

		ma_uint64 frameCount = 0;
		//ma_result result = ma_decoder_get_length_in_pcm_frames(m_Decoder, &frameCount); //MISSING

		// if (result != MA_SUCCESS) {
	 //        std::cerr << "Audio: Failed to get frame count - Error: " << result << std::endl;
	 //        return 0;
	 //    }

		return frameCount;
	}

	float Audio::GetDurationSeconds() const {
		if (!m_IsLoaded) {
			return 0.0f;
		}

		uint64_t frameCount = GetFrameCount();
		uint32_t sampleRate = GetSampleRate();

		if (sampleRate == 0) {
			return 0.0f;
		}

		return static_cast<float>(frameCount) / static_cast<float>(sampleRate);
	}

	void Audio::Cleanup() {
		if (m_IsLoaded) {
			ma_decoder_uninit(&m_Decoder);
			m_IsLoaded = false;
			m_Filepath.clear();
			std::memset(&m_Decoder, 0, sizeof(ma_decoder));
		}
	}



}