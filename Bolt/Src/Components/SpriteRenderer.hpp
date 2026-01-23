#pragma once
#include "../Graphics/TextureHandle.hpp"

namespace Bolt {
	struct SpriteRenderer {
		SpriteRenderer() = default;

		short SortingOrder{0};
		uint8_t SortingLayer{0};
		TextureHandle TextureHandle;
		Color Color{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}