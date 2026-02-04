#include "pch.hpp"
#include "TextureManager.hpp"

namespace Bolt {
	std::array<std::string, 9> TextureManager::s_DefaultTextures = {
		   "../Assets/Textures/Square.png",
		   "../Assets/Textures/Pixel.png",
		   "../Assets/Textures/Circle.png",
		   "../Assets/Textures/Capsule.png",
		   "../Assets/Textures/IsometricDiamond.png",
		   "../Assets/Textures/HexagonFlatTop.png",
		   "../Assets/Textures/HexagonPointedTop.png",
		   "../Assets/Textures/9sliced.png",
			"../Assets/Textures/Invisible.png"
	};

	std::vector<TextureEntry> TextureManager::s_Textures = {};
	std::queue<uint16_t> TextureManager::s_FreeIndices = {};

	bool TextureManager::s_IsInitialized = false;
	constexpr uint16_t k_InvalidIndex = std::numeric_limits<uint16_t>::max();

	void TextureManager::Initialize() {
		BOLT_ASSERT(!s_IsInitialized, BoltErrorCode::AlreadyInitialized, "TextureManager is already initialized");

		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		s_IsInitialized = true;
		LoadDefaultTextures();
	}

	void TextureManager::Shutdown() {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		UnloadAll(true);
		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		s_IsInitialized = false;
	}

	TextureHandle TextureManager::LoadTexture(const std::string& path, Filter filter, Wrap u, Wrap v) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");
		BOLT_ASSERT(File::Exists(path), BoltErrorCode::FileNotFound, "File with path '" + path + "' doesn't exist");

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

			BOLT_ASSERT(entry.Texture.IsValid(), BoltErrorCode::LoadFailed, "Failed to load texture with path '" + path + "'");

			entry.Generation++;
			entry.IsValid = true;
			entry.Name = path;
		}
		else {
			index = static_cast<uint16_t>(s_Textures.size());

			TextureEntry entry;
			entry.Texture = Texture2D(path.c_str(), filter, u, v);

			BOLT_ASSERT(entry.Texture.IsValid(),BoltErrorCode::LoadFailed, "Failed to load texture with path '" + path + "'");

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = path;

			s_Textures.push_back(std::move(entry));
		}

		return { index, s_Textures[index].Generation };
	}

	TextureHandle TextureManager::GetDefaultTexture(DefaultTexture type) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		int index = static_cast<int>(type);

		BOLT_ASSERT(index >= 0 || index < static_cast<int>(s_DefaultTextures.size()), BoltErrorCode::OutOfRange, "Invalid DefaultTexture! Index out of range");

		return TextureHandle(static_cast<uint16_t>(index), s_Textures[index].Generation);
	}

	void TextureManager::UnloadTexture(TextureHandle blockTexture) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		BOLT_ASSERT(blockTexture.index < s_Textures.size(), BoltErrorCode::InvalidHandle,
			"Invalid texture handle index: " + std::to_string(blockTexture.index));

		TextureEntry& entry = s_Textures[blockTexture.index];

		BOLT_ASSERT(entry.IsValid && entry.Generation == blockTexture.generation, BoltErrorCode::InvalidHandle,
        "Texture handle is outdated or invalid");

		BOLT_ASSERT(blockTexture.index >= s_DefaultTextures.size(), BoltErrorCode::Undefined, "Cannot unload default texture");

		entry.Texture.Destroy();
		entry.IsValid = false;
		entry.Name.clear();
		s_FreeIndices.push(blockTexture.index);
	}

	TextureHandle TextureManager::GetTextureHandle(const std::string& name) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		auto blockTexture = FindTextureByPath(name);

		BOLT_ASSERT(blockTexture.index != k_InvalidIndex, BoltErrorCode::InvalidArgument,
			"Texture with name " + name + " doesn't exist.");

		return blockTexture;
	}

	std::vector<TextureHandle> TextureManager::GetLoadedHandles() {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		std::vector<TextureHandle> handles;
		handles.reserve(s_Textures.size());

		for (size_t i = 0; i < s_Textures.size(); i++) {
			if (s_Textures[i].IsValid) {
				handles.emplace_back(static_cast<uint16_t>(i), s_Textures[i].Generation);
			}
		}

		return handles;
	}

	Texture2D* TextureManager::GetTexture(TextureHandle blockTexture) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		BOLT_ASSERT(blockTexture.index < s_Textures.size(), BoltErrorCode::OutOfRange,
			"TextureHandle index " + std::to_string(blockTexture.index) +
			" out of range (max: " + std::to_string(s_Textures.size() - 1) + ")"
		);

		TextureEntry& entry = s_Textures[blockTexture.index];

		BOLT_ASSERT(entry.IsValid, BoltErrorCode::InvalidHandle, "Texture at index " + std::to_string(blockTexture.index) + " is not valid");

		BOLT_ASSERT(entry.Generation == blockTexture.generation, BoltErrorCode::InvalidHandle,
			"Invalid texture entry: entry generation " + std::to_string(entry.Generation) +
			" != handle generation " + std::to_string(blockTexture.generation));

		return &entry.Texture;
	}

	void TextureManager::LoadDefaultTextures() {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		s_Textures.reserve(s_DefaultTextures.size() + 32);

		for (const auto& texPath : s_DefaultTextures) {
			TextureEntry entry;
			entry.Texture = Texture2D(texPath.c_str(), Filter::Point, Wrap::Clamp, Wrap::Clamp);

			BOLT_ASSERT(entry.Texture.IsValid(), BoltErrorCode::LoadFailed, "Failed to load default texture at path: " + texPath);

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = texPath;

			s_Textures.push_back(std::move(entry));
		}
	}

	void TextureManager::UnloadAll(bool defaultTextures) {
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

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
		BOLT_ASSERT(s_IsInitialized, BoltErrorCode::NotInitialized, "TextureManager isn't initialized");

		for (size_t i = 0; i < s_Textures.size(); i++) {
			const TextureEntry& entry = s_Textures[i];
			if (entry.IsValid && entry.Name == path) {
				return TextureHandle(static_cast<uint16_t>(i), entry.Generation);
			}
		}

		return { k_InvalidIndex, 0 };
	}
}