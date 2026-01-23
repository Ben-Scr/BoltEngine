#pragma once
#include <miniaudio/miniaudio.h>

namespace Bolt {

    class Audio {
    public:
        Audio() = default;
        ~Audio();

        Audio(const Audio&) = delete;
        Audio& operator=(const Audio&) = delete;
        Audio(Audio&& other) noexcept;
        Audio& operator=(Audio&& other) noexcept;

        bool LoadFromFile(const std::string& filepath);
       
        const ma_decoder& GetDecoder() const { return m_Decoder; }
        bool IsLoaded() const { return m_IsLoaded; }
        const std::string& GetFilepath() const { return m_Filepath; }


        uint32_t GetSampleRate() const;
        uint32_t GetChannels() const;
        uint64_t GetFrameCount() const;
        float GetDurationSeconds() const;

    private:
        ma_decoder m_Decoder{};
        bool m_IsLoaded = false;
        std::string m_Filepath;

        void Cleanup();
    };
}