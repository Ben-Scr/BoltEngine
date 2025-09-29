#pragma once
#include "Texture2D.hpp"
#include "TextureHandle.hpp"

#include <vector>
#include <queue>
#include <memory>
#include <string>

namespace Bolt {
    class Application;
}

namespace bgfx {
    struct TextureHandle;
}

namespace Bolt {
 /*   struct TextureEntry {
        TextureEntry() = default;
        TextureEntry(TextureEntry&&) noexcept = default;
        TextureEntry& operator=(TextureEntry&&) noexcept = default;
        TextureEntry(const TextureEntry&) = delete;
        TextureEntry& operator=(const TextureEntry&) = delete;

        Texture2D Texture;
        uint16_t Generation = 0;
        std::string Name;
        bool IsValid = false;
    };

    enum class  DefaultTexture : uint8_t {
        Square,
        Circle,
        Capsule,
        IsometricDiamond,
        HexagonFlatTop,
        HexagonPointedTop,
        _9Sliced,
    };

    class TextureManager {
    public:
        static void Initialize();
        static void Shutdown();

        static TextureHandle LoadTexture(const std::string& path, Filter filter = Filter::Point, Wrap u = Wrap::Clamp, Wrap v = Wrap::Clamp);
        static TextureHandle GetDefaultTexture(DefaultTexture type);
        static void UnloadTexture(TextureHandle handle);
        static TextureHandle GetTextureHandle(const std::string& name);
        static Texture2D& GetTexture(TextureHandle handle);
        static std::vector<TextureHandle> GetLoadedHandles();
        static void UnloadAll(bool defaultTextures = false);


        static bool IsValid(TextureHandle handle) {
            return handle.index < s_Textures.size() &&
                s_Textures[handle.index].IsValid &&
                s_Textures[handle.index].Generation == handle.generation;
        }


    private:
        static TextureHandle FindTextureByPath(const std::string& path);
        static void LoadDefaultTextures();

        static std::string s_DefaultTextures[7];
        static std::vector<TextureEntry> s_Textures;
        static std::queue<uint16_t> s_FreeIndices;
        static bool s_IsInitialized;

        friend class Renderer2D;
    };*/
}

namespace std {
    template<>
    struct hash<Bolt::TextureHandle> {
        size_t operator()(const Bolt::TextureHandle& h) const noexcept {
            return (static_cast<size_t>(h.index) << 16) ^ static_cast<size_t>(h.generation);
        }
    };
}