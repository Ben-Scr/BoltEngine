#pragma once
#include "../Graphics/TextureHandle.hpp"
#include "../Collections/Color.hpp"

namespace Bolt {
	struct GuiImage {
		TextureHandle TextureHandle;
		Color Color;
	};
}