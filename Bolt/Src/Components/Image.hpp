#pragma once
#include "Graphics/TextureHandle.hpp"
#include "Collections/Color.hpp"

namespace Bolt {
	struct ImageComponent {
		TextureHandle TextureHandle;
		Color Color;
	};
}