#pragma once
#include "../Graphics/TextureHandle.hpp"
#include "../Collections/Color.hpp"

namespace Bolt {
	struct Image {
		TextureHandle TextureHandle;
		Color Color;
	};
}