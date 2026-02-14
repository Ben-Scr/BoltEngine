#pragma once

namespace Bolt {
    struct BOLT_API TextureHandle {
        uint16_t index;
        uint16_t generation;

        static constexpr uint16_t k_InvalidIndex = std::numeric_limits<uint16_t>::max();
        static TextureHandle Invalid() { return TextureHandle(k_InvalidIndex, 0); }

        TextureHandle(uint16_t index, uint16_t generation) : index{ index }, generation{ generation } {}
        TextureHandle() : index{ 0 }, generation{ 0 } {}

        bool IsValid() const { return index != UINT16_MAX; }

        bool operator==(const TextureHandle& other) const {
            return index == other.index && generation == other.generation;
        }

        bool operator!=(const TextureHandle& other) const {
            return !(*this == other);
        }
    };
}