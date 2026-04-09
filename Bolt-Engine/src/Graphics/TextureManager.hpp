#pragma once
#include "Texture2D.hpp"
#include "TextureHandle.hpp"
#include "Core/Export.hpp"

#include "Serialization/Path.hpp"

#include <vector>
#include <queue>
#include <memory>
#include <string>
#include <array>
#include <cstdint>

namespace Bolt {
    class Application;
}

namespace Bolt {
        struct TextureEntry {
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

        enum class BOLT_API DefaultTexture : uint8_t {
            Square,
            Pixel,
            Circle,
            Capsule,
            IsometricDiamond,
            HexagonFlatTop,
            HexagonPointedTop,
            _9Sliced,
            Invisible
        };

        class BOLT_API TextureManager {
        public:
            static void Initialize();
            static void Shutdown();

            static TextureHandle LoadTexture(const std::string_view& path, Filter filter = Filter::Point, Wrap u = Wrap::Clamp, Wrap v = Wrap::Clamp);
            static TextureHandle GetDefaultTexture(DefaultTexture type);
            static void UnloadTexture(TextureHandle handle);
            static TextureHandle GetTextureHandle(const std::string& name);
            static Texture2D* GetTexture(TextureHandle handle);
            static std::vector<TextureHandle> GetLoadedHandles();
            static void UnloadAll(bool defaultTextures = false);

            /// Returns the texture path relative to a texture root directory.
            /// This is the same format accepted by LoadTexture().
            static std::string GetTextureName(TextureHandle handle) {
                if (handle.index >= s_Textures.size() || !s_Textures[handle.index].IsValid
                    || s_Textures[handle.index].Generation != handle.generation)
                    return "";

                const std::string& fullName = s_Textures[handle.index].Name;

                // Try stripping any known texture root prefix
                auto tryStrip = [&](const std::string& root) -> std::string {
                    if (root.empty() || fullName.size() <= root.size()) return "";
                    if (fullName.compare(0, root.size(), root) != 0) return "";
                    size_t start = root.size();
                    if (start < fullName.size() && (fullName[start] == '/' || fullName[start] == '\\'))
                        start++;
                    return fullName.substr(start);
                };

                // Try primary root first
                std::string rel = tryStrip(s_RootPath);
                if (!rel.empty()) return rel;

                // Try alternative roots (Assets/Textures, BoltAssets/Textures)
                std::string base = Path::ExecutableDir();
                rel = tryStrip(Path::Combine(base, "Assets", "Textures"));
                if (!rel.empty()) return rel;
                rel = tryStrip(Path::Combine(base, "BoltAssets", "Textures"));
                if (!rel.empty()) return rel;

                return fullName;
            }

            static bool IsValid(TextureHandle handle) {
                return handle.index < s_Textures.size() &&
                    s_Textures[handle.index].IsValid &&
                    s_Textures[handle.index].Generation == handle.generation;
            }

        private:
            static TextureHandle FindTextureByPath(const std::string& path);
            static void LoadDefaultTextures();

            static std::array<std::string, 9> s_DefaultTextures;
            static std::vector<TextureEntry> s_Textures;
            static std::queue<uint16_t> s_FreeIndices;
            static bool s_IsInitialized;

            static std::string s_RootPath;

            friend class Renderer2D;
        };
}

namespace std {
    template<>
    struct hash<Bolt::TextureHandle> {
        size_t operator()(const Bolt::TextureHandle& h) const noexcept {
            return (static_cast<size_t>(h.index) << 16) ^ static_cast<size_t>(h.generation);
        }
    };
}