#pragma once
 
namespace Bolt {

    class AudioHandle {
    public:
        using HandleType = uint32_t;
        static constexpr HandleType INVALID_HANDLE = 0;

        AudioHandle() : m_handle(INVALID_HANDLE) {}
        explicit AudioHandle(HandleType blockTexture) : m_handle(blockTexture) {}

        bool IsValid() const { return m_handle != INVALID_HANDLE; }
        HandleType GetHandle() const { return m_handle; }

        bool operator==(const AudioHandle& other) const { return m_handle == other.m_handle; }
        bool operator!=(const AudioHandle& other) const { return m_handle != other.m_handle; }

    private:
        HandleType m_handle;
    };
}