#include "../pch.hpp"
#include "TextureManager.hpp"

namespace Bolt {
	std::array<std::string, 8> TextureManager::s_DefaultTextures = {
		   "Assets/Textures/Square.png",
		   "Assets/Textures/Pixel.png",
		   "Assets/Textures/Circle.png",
		   "Assets/Textures/Capsule.png",
		   "Assets/Textures/IsometricDiamond.png",
		   "Assets/Textures/HexagonFlatTop.png",
		   "Assets/Textures/HexagonPointedTop.png",
		   "Assets/Textures/9sliced.png"
	};

	std::vector<TextureEntry> TextureManager::s_Textures = {};
	std::queue<uint16_t> TextureManager::s_FreeIndices = {};

	bool TextureManager::s_IsInitialized = false;
	constexpr uint16_t k_InvalidIndex = std::numeric_limits<uint16_t>::max();

	void TextureManager::Initialize() {
		if (s_IsInitialized)
		{
			Logger::Warning("TextureManager", "Already Initialized");
			return;
		}

		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		LoadDefaultTextures();
		s_IsInitialized = true;
	}

	void TextureManager::Shutdown() {
		if (!s_IsInitialized)
			return;

		UnloadAll(true);
		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		s_IsInitialized = false;
	}

	TextureHandle TextureManager::LoadTexture(const std::string& path, Filter filter, Wrap u, Wrap v) {
		if (!s_IsInitialized) {
			Logger::Error("TextureManager", "Call TextureManager::Initialize() before loading textures.");
			return { k_InvalidIndex, 0 };
		}

		auto existingHandle = FindTextureByPath(path);
		if (existingHandle.index != k_InvalidIndex) {
			return existingHandle;
		}

		uint16_t index = k_InvalidIndex;
		if (!s_FreeIndices.empty()) {
			index = s_FreeIndices.front();
			s_FreeIndices.pop();

			auto& entry = s_Textures[index];
			entry.Texture.Destroy();
			entry.Texture = Texture2D(path.c_str(), filter, u, v);

			if (!entry.Texture.IsValid()) {
				Logger::Error("TextureManager", "Failed to load texture at path: " + path);
				entry.IsValid = false;
				s_FreeIndices.push(index);
				return { k_InvalidIndex, 0 };
			}

			entry.Generation++;
			entry.IsValid = true;
			entry.Name = path;
		}
		else {
			index = static_cast<uint16_t>(s_Textures.size());

			TextureEntry entry;
			entry.Texture = Texture2D(path.c_str(), filter, u, v);

			if (!entry.Texture.IsValid()) {
				Logger::Error("TextureManager", "Failed to load texture at path: " + path);
				return { k_InvalidIndex, 0 };
			}

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = path;

			s_Textures.push_back(std::move(entry));
		}

		return { index, s_Textures[index].Generation };
	}

	TextureHandle TextureManager::GetDefaultTexture(DefaultTexture type) {
		int index = static_cast<int>(type);

		if (index < 0 || index >= static_cast<int>(s_DefaultTextures.size())) {
			Logger::Warning("TextureManager", "Invalid DefaultTexture enum index");
		}

		if (!s_IsInitialized) {
			Logger::Warning("TextureManager", "Default textures not loaded. Call TextureManager::Initialize() first.");
		}

		return TextureHandle(static_cast<uint16_t>(index), s_Textures[index].Generation);
	}

	void TextureManager::UnloadTexture(TextureHandle blockTexture) {
		if (blockTexture.index >= s_Textures.size()) {
			Logger::Warning("TextureManager", "Invalid texture handle index: " + std::to_string(blockTexture.index));
			return;
		}

		TextureEntry& entry = s_Textures[blockTexture.index];

		if (!entry.IsValid || entry.Generation != blockTexture.generation) {
			Logger::Warning("TextureManager", "Texture handle is outdated or invalid");
			return;
		}

		if (blockTexture.index < s_DefaultTextures.size()) {
			Logger::Warning("TextureManager", "Cannot unload default texture");
			return;
		}

		entry.Texture.Destroy();
		entry.IsValid = false;
		entry.Name.clear();
		s_FreeIndices.push(blockTexture.index);
	}

	TextureHandle TextureManager::GetTextureHandle(const std::string& name) {
		auto blockTexture = FindTextureByPath(name);

		if (blockTexture.index == k_InvalidIndex) {
			Logger::Warning("TextureManager", "Texture with name " + name + " doesn't exist.");
		}

		return blockTexture;
	}

	std::vector<TextureHandle> TextureManager::GetLoadedHandles() {
		std::vector<TextureHandle> handles;
		handles.reserve(s_Textures.size());

		for (size_t i = 0; i < s_Textures.size(); i++) {
			if (s_Textures[i].IsValid) {
				handles.emplace_back(static_cast<uint16_t>(i), s_Textures[i].Generation);
			}
		}

		return handles;
	}

	Texture2D& TextureManager::GetTexture(TextureHandle blockTexture) {
		if (blockTexture.index >= s_Textures.size()) {
			Logger::Warning("TextureManager",
				"TextureHandle index " + std::to_string(blockTexture.index) +
				" out of range (max: " + std::to_string(s_Textures.size() - 1) + ")"
			);
		}

		TextureEntry& entry = s_Textures[blockTexture.index];

		if (!entry.IsValid) {
			Logger::Warning("TextureManager", "Texture at index " + std::to_string(blockTexture.index) + " is not valid");
		}

		if (entry.Generation != blockTexture.generation) {
			Logger::Warning("TextureManager",
				"Invalid texture entry: entry generation " + std::to_string(entry.Generation) +
				" != handle generation " + std::to_string(blockTexture.generation)
			);
		}

		return entry.Texture;
	}

	void TextureManager::LoadDefaultTextures() {
		s_Textures.reserve(s_DefaultTextures.size() + 32);

		for (const auto& texPath : s_DefaultTextures) {
			TextureEntry entry;
			entry.Texture = Texture2D(texPath.c_str(), Filter::Point, Wrap::Clamp, Wrap::Clamp);

			if (!entry.Texture.IsValid()) {
				Logger::Warning("TextureManager", "Failed to load default texture at path: " + texPath);
			}

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = texPath;

			s_Textures.push_back(std::move(entry));
		}
	}

	void TextureManager::UnloadAll(bool defaultTextures) {
		size_t startOffset = defaultTextures ? 0 : s_DefaultTextures.size();
		for (size_t i = startOffset; i < s_Textures.size(); i++) {
			if (s_Textures[i].IsValid) {
				s_Textures[i].Texture.Destroy();
				s_Textures[i].IsValid = false;
				s_Textures[i].Name.clear();
				if (i >= s_DefaultTextures.size()) {
					s_FreeIndices.push(static_cast<uint16_t>(i));
				}
			}
		}

		if (defaultTextures) {
			s_Textures.clear();
			while (!s_FreeIndices.empty()) {
				s_FreeIndices.pop();
			}
		}
	}

	TextureHandle TextureManager::FindTextureByPath(const std::string& path) {
		for (size_t i = 0; i < s_Textures.size(); i++) {
			const TextureEntry& entry = s_Textures[i];
			if (entry.IsValid && entry.Name == path) {
				return TextureHandle(static_cast<uint16_t>(i), entry.Generation);
			}
		}

		return { k_InvalidIndex, 0 };
	}
}