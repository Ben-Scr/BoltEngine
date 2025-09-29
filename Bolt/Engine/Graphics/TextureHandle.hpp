#pragma once

namespace Bolt {
    struct TextureHandle {
        uint16_t index;
        uint16_t generation;

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