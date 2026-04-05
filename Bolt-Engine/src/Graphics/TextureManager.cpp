#include "pch.hpp"
#include "TextureManager.hpp"
#include <Serialization/File.hpp>

namespace Bolt {
	std::array<std::string, 9> TextureManager::s_DefaultTextures = {
		   "Default/Square.png",
		   "Default/Pixel.png",
		   "Default/Circle.png",
		   "Default/Capsule.png",
		   "Default/IsometricDiamond.png",
		   "Default/HexagonFlatTop.png",
		   "Default/HexagonPointedTop.png",
		   "Default/9sliced.png",
		   "Default/Invisible.png"
	};

	std::vector<TextureEntry> TextureManager::s_Textures = {};
	std::queue<uint16_t> TextureManager::s_FreeIndices = {};

	bool TextureManager::s_IsInitialized = false;
	std::string TextureManager::s_RootPath = Path::Combine("Assets", "Textures");

	constexpr uint16_t k_InvalidIndex = std::numeric_limits<uint16_t>::max();

	void TextureManager::Initialize() {
		if (s_IsInitialized) {
			BT_CORE_WARN("TextureManager is already initialized");
			return;
		}

		s_RootPath = Path::Combine(Path::ExecutableDir(), "Assets", "Textures");

		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		s_IsInitialized = true;
		LoadDefaultTextures();
	}

	void TextureManager::Shutdown() {
		if (!s_IsInitialized) {
			BT_CORE_WARN("TextureManager isn't initialized");
			return;
		}

		UnloadAll(true);
		s_Textures.clear();
		while (!s_FreeIndices.empty()) {
			s_FreeIndices.pop();
		}

		s_IsInitialized = false;
	}

	TextureHandle TextureManager::LoadTexture(const std::string_view& path, Filter filter, Wrap u, Wrap v) {
		const std::string fullpath = Path::Combine(s_RootPath, path);

		if (!s_IsInitialized) {
			BT_CORE_ERROR("[{}] TextureManager isn't initialized", ErrorCodeToString(BoltErrorCode::NotInitialized));
			return TextureHandle::Invalid();
		}
		if (!File::Exists(fullpath)) {
			BT_CORE_ERROR("[{}] File with path '{}' doesn't exist", ErrorCodeToString(BoltErrorCode::FileNotFound), fullpath);
			return TextureHandle::Invalid();
		}

		auto existingHandle = FindTextureByPath(fullpath);
		if (existingHandle.index != k_InvalidIndex) {
			return existingHandle;
		}

		uint16_t index = k_InvalidIndex;
		if (!s_FreeIndices.empty()) {
			index = s_FreeIndices.front();
			s_FreeIndices.pop();

			auto& entry = s_Textures[index];
			entry.Texture.Destroy();
			entry.Texture = Texture2D(fullpath.c_str(), filter, u, v);

			if (!entry.Texture.IsValid()) {
				BT_CORE_ERROR("[{}] Failed to load texture with path '{}'", ErrorCodeToString(BoltErrorCode::LoadFailed), fullpath);
				return TextureHandle::Invalid();
			}

			entry.Generation++;
			entry.IsValid = true;
			entry.Name = fullpath;
		}
		else {
			index = static_cast<uint16_t>(s_Textures.size());

			TextureEntry entry;
			entry.Texture = Texture2D(fullpath.c_str(), filter, u, v);

			if (!entry.Texture.IsValid()) {
				BT_CORE_ERROR("[{}] Failed to load texture with path '{}'", ErrorCodeToString(BoltErrorCode::LoadFailed), fullpath);
				return TextureHandle::Invalid();
			}

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = fullpath;

			s_Textures.push_back(std::move(entry));
		}

		return { index, s_Textures[index].Generation };
	}

	TextureHandle TextureManager::GetDefaultTexture(DefaultTexture type) {
		if (!s_IsInitialized) {
			BT_CORE_ERROR("[{}] TextureManager isn't initialized", ErrorCodeToString(BoltErrorCode::NotInitialized));
			return TextureHandle::Invalid();
		}

		int index = static_cast<int>(type);

		if (index < 0 || index >= static_cast<int>(s_DefaultTextures.size())) {
			BT_CORE_ERROR("[{}] Invalid DefaultTexture index {}", ErrorCodeToString(BoltErrorCode::OutOfRange), index);
			return TextureHandle::Invalid();
		}

		return TextureHandle(static_cast<uint16_t>(index), s_Textures[index].Generation);
	}

	void TextureManager::UnloadTexture(TextureHandle blockTexture) {
		if (!s_IsInitialized) {
			BT_CORE_WARN("TextureManager isn't initialized");
			return;
		}

		if (blockTexture.index >= s_Textures.size()) {
			BT_CORE_WARN("[{}] Invalid texture handle index: {}", ErrorCodeToString(BoltErrorCode::InvalidHandle), blockTexture.index);
			return;
		}

		TextureEntry& entry = s_Textures[blockTexture.index];

		if (!entry.IsValid || entry.Generation != blockTexture.generation) {
			BT_CORE_WARN("[{}] Texture handle is outdated or invalid", ErrorCodeToString(BoltErrorCode::InvalidHandle));
			return;
		}

		if (blockTexture.index < s_DefaultTextures.size()) {
			BT_CORE_WARN("Cannot unload default texture");
			return;
		}

		entry.Texture.Destroy();
		entry.IsValid = false;
		entry.Name.clear();
		s_FreeIndices.push(blockTexture.index);
	}

	TextureHandle TextureManager::GetTextureHandle(const std::string& name) {
		if (!s_IsInitialized) {
			BT_CORE_ERROR("[{}] TextureManager isn't initialized", ErrorCodeToString(BoltErrorCode::NotInitialized));
			return TextureHandle::Invalid();
		}

		auto blockTexture = FindTextureByPath(name);

		if (blockTexture.index == k_InvalidIndex) {
			BT_CORE_WARN("[{}] Texture with name '{}' doesn't exist", ErrorCodeToString(BoltErrorCode::InvalidArgument), name);
			return TextureHandle::Invalid();
		}

		return blockTexture;
	}

	std::vector<TextureHandle> TextureManager::GetLoadedHandles() {
		if (!s_IsInitialized) {
			BT_CORE_WARN("TextureManager isn't initialized");
			return {};
		}

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
		if (!s_IsInitialized) {
			BT_CORE_ERROR("[{}] TextureManager isn't initialized", ErrorCodeToString(BoltErrorCode::NotInitialized));
			return nullptr;
		}

		if (blockTexture.index >= s_Textures.size()) {
			BT_CORE_ERROR("[{}] TextureHandle index {} out of range", ErrorCodeToString(BoltErrorCode::OutOfRange), blockTexture.index);
			return nullptr;
		}

		TextureEntry& entry = s_Textures[blockTexture.index];

		if (!entry.IsValid) {
			BT_CORE_ERROR("[{}] Texture at index {} is not valid", ErrorCodeToString(BoltErrorCode::InvalidHandle), blockTexture.index);
			return nullptr;
		}

		if (entry.Generation != blockTexture.generation) {
			BT_CORE_ERROR("[{}] Invalid texture generation: entry {} != handle {}", ErrorCodeToString(BoltErrorCode::InvalidHandle), entry.Generation, blockTexture.generation);
			return nullptr;
		}

		return &entry.Texture;
	}

	void TextureManager::LoadDefaultTextures() {
		if (!s_IsInitialized) {
			BT_CORE_ERROR("[{}] TextureManager isn't initialized", ErrorCodeToString(BoltErrorCode::NotInitialized));
			return;
		}

		s_Textures.reserve(s_DefaultTextures.size() + 32);

		for (const auto& texPath : s_DefaultTextures) {
			TextureEntry entry;
			entry.Texture = Texture2D(Path::Combine(s_RootPath, texPath).c_str(), Filter::Point, Wrap::Clamp, Wrap::Clamp);

			if (!entry.Texture.IsValid()) {
				BT_CORE_ERROR("[{}] Failed to load default texture at path: {}", ErrorCodeToString(BoltErrorCode::LoadFailed), texPath);
				s_Textures.push_back(std::move(entry));
				continue;
			}

			entry.Generation = 0;
			entry.IsValid = true;
			entry.Name = texPath;

			s_Textures.push_back(std::move(entry));
		}
	}

	void TextureManager::UnloadAll(bool defaultTextures) {
		if (!s_IsInitialized) {
			BT_CORE_WARN("TextureManager isn't initialized");
			return;
		}

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
		if (!s_IsInitialized) {
			return TextureHandle::Invalid();
		}

		for (size_t i = 0; i < s_Textures.size(); i++) {
			const TextureEntry& entry = s_Textures[i];
			if (entry.IsValid && entry.Name == path) {
				return TextureHandle(static_cast<uint16_t>(i), entry.Generation);
			}
		}

		return { k_InvalidIndex, 0 };
	}
}