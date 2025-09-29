#include "../pch.hpp"
#include "TextureManager.hpp"
#include "../Debug/Logger.hpp"

namespace Bolt {
 //   std::vector<TextureEntry> TextureManager::s_Textures = {};
 //   std::queue<uint16_t> TextureManager::s_FreeIndices = {};
 //   bool TextureManager::s_IsInitialized = false;
 //   std::string TextureManager::s_DefaultTextures[7] = {
 //       "Assets/Textures/Square.png",
 //       "Assets/Textures/Circle.png",
 //       "Assets/Textures/Capsule.png",
 //       "Assets/Textures/IsometricDiamond.png",
 //       "Assets/Textures/HexagonFlatTop.png",
 //       "Assets/Textures/HexagonPointedTop.png",
 //       "Assets/Textures/9sliced.png"
 //   };

 //   void TextureManager::Initialize() {
	//	LoadDefaultTextures();
 //   }
 //   void TextureManager::Shutdown() {
 //       UnloadAll(true);
 //       s_Textures.clear();
 //       s_FreeIndices = {};
 //       s_IsInitialized = false;
	//}

 //   TextureHandle TextureManager::LoadTexture(const std::string& path, Filter filter, Wrap u, Wrap v) {
 //       if (!s_IsInitialized) {
 //           Logger::Error("Call LoadTexture(" + path + ") after creating the Application.");
 //           return {};
 //       }

 //       auto existingHandle = FindTextureByPath(path);
 //       if (existingHandle.index != UINT16_MAX) {
 //           return existingHandle;
 //       }

 //       uint16_t index;
 //       if (!s_FreeIndices.empty()) {
 //           index = s_FreeIndices.front();
 //           s_FreeIndices.pop();
 //           auto& entry = s_Textures[index];

 //           entry.Texture.Destroy();

 //           entry.Texture = Texture2D(path.c_str(), filter, u, v);

 //           if (!entry.Texture.IsValid()) {
 //               Logger::Error("Failed to load texture: " + path);
 //               s_FreeIndices.push(index);
 //               return {};
 //           }

 //           entry.Generation++;
 //           entry.IsValid = true;
 //           entry.Name = path;
 //       }
 //       else {
 //           index = static_cast<uint16_t>(s_Textures.size());

 //           TextureEntry entry;
 //           entry.Texture = Texture2D(path.c_str(), filter, u, v);

 //           if (!entry.Texture.IsValid()) {
 //               Logger::Error("Failed to load texture: " + path);
 //               return {};
 //           }

 //           entry.Generation = 0;
 //           entry.IsValid = true;
 //           entry.Name = path;

 //           s_Textures.push_back(std::move(entry));
 //       }

 //       return { index, s_Textures[index].Generation };
 //   }

 //   TextureHandle TextureManager::GetDefaultTexture(DefaultTexture type) {
 //       int index = static_cast<int>(type);

 //       if (index < 0 || index >= std::size(s_DefaultTextures)) {
 //           throw std::runtime_error("Invalid DefaultTexture enum index");
 //       }

 //       if (!s_IsInitialized) {
 //           throw std::runtime_error("Default textures not loaded. Call Application constructor first.");
 //       }

 //       return TextureHandle(index, s_Textures[index].Generation);
 //   }

 //   void TextureManager::UnloadTexture(TextureHandle handle) {
 //       if (handle.index >= s_Textures.size()) {
 //           Logger::Warning("Invalid texture handle index: " + std::to_string(handle.index));
 //           return;
 //       }

 //       TextureEntry& entry = s_Textures[handle.index];


 //       if (!entry.IsValid || entry.Generation != handle.generation) {
 //           Logger::Warning("Texture handle is outdated or invalid");
 //           return;
 //       }

 //       if (handle.index < std::size(s_DefaultTextures)) {
 //           Logger::Warning("Cannot unload default texture");
 //           return;
 //       }

 //       entry.Texture.Destroy();
 //       entry.IsValid = false;
 //       entry.Name.clear();
 //       s_FreeIndices.push(handle.index);
 //   }

 //   TextureHandle TextureManager::GetTextureHandle(const std::string& name) {
 //       auto handle = FindTextureByPath(name);

 //       if (handle.index == UINT16_MAX) {
 //           throw std::runtime_error("Texture with name " + name + " doesn't exist.");
 //       }

 //       return handle;
 //   }

 //   std::vector<TextureHandle> TextureManager::GetLoadedHandles() {
 //       std::vector<TextureHandle> handles;
 //       handles.reserve(s_Textures.size() - std::size(s_DefaultTextures));

 //       for (size_t i = std::size(s_DefaultTextures); i < s_Textures.size(); i++) {
 //           if (s_Textures[i].IsValid) {
 //               handles.emplace_back(static_cast<uint16_t>(i), s_Textures[i].Generation);
 //           }
 //       }

 //       return handles;
 //   }

 //   Texture2D& TextureManager::GetTexture(TextureHandle handle) {
 //       if (handle.index >= s_Textures.size()) {
 //           throw std::runtime_error(
 //               "TextureHandle index " + std::to_string(handle.index) +
 //               " out of range (max: " + std::to_string(s_Textures.size() - 1) + ")"
 //           );
 //       }

 //       TextureEntry& entry = s_Textures[handle.index];

 //       if (!entry.IsValid) {
 //           throw std::runtime_error("Texture at index " + std::to_string(handle.index) + " is not valid");
 //       }

 //       if (entry.Generation != handle.generation) {
 //           throw std::runtime_error(
 //               "Invalid textureEntry: entry generation " + std::to_string(entry.Generation) +
 //               " != handle generation " + std::to_string(handle.generation)
 //           );
 //       }

 //       return entry.Texture;
 //   }

 //   void TextureManager::LoadDefaultTextures() {
 //       if (s_IsInitialized) {
 //           Logger::Warning("Default textures already loaded");
 //           return;
 //       }

 //       s_IsInitialized = true;
 //       s_Textures.reserve(std::size(s_DefaultTextures) + 32);

 //       for (const auto& texPath : s_DefaultTextures) {
 //           TextureEntry entry;
 //           entry.Texture = Texture2D(texPath.c_str(), Filter::Point, Wrap::Clamp, Wrap::Clamp);

 //           if (!entry.Texture.IsValid()) {
 //               throw std::runtime_error("Failed to load default texture: " + texPath);
 //           }

 //           entry.Generation = 0;
 //           entry.IsValid = true;
 //           entry.Name = texPath;

 //           s_Textures.push_back(std::move(entry));
 //       }
 //   }

	//// Note: when defaultTextures is false, only non-default textures are unloaded
 //   void TextureManager::UnloadAll(bool defaultTextures) {
 //       size_t startOffset = defaultTextures ? 0 : std::size(s_DefaultTextures);
 //       for (size_t i = startOffset; i < s_Textures.size(); i++) {
 //           if (s_Textures[i].IsValid) {
 //               s_Textures[i].Texture.Destroy();
 //               s_Textures[i].IsValid = false;
 //               s_Textures[i].Name.clear();
 //           }
 //       }

 //       s_Textures.erase(s_Textures.begin() + startOffset,s_Textures.end());

 //       std::queue<uint16_t> empty;
 //       std::swap(s_FreeIndices, empty);
 //   }

 //   TextureHandle TextureManager::FindTextureByPath(const std::string& path) {
 //       for (size_t i = 0; i < s_Textures.size(); i++) {
 //           if (s_Textures[i].IsValid && s_Textures[i].Name == path) {
 //               return TextureHandle(static_cast<uint16_t>(i), s_Textures[i].Generation);
 //           }
 //       }
 //       return TextureHandle(UINT16_MAX, 0);
 //   }
}