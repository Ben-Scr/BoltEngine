#pragma once

#include "Graphics/Texture2D.hpp"

#include <cstdint>
#include <string>

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

} // namespace Bolt
