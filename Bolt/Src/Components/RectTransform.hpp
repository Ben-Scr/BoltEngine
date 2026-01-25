#pragma once
#include "../Collections/Vec2.hpp"

namespace Bolt {
	struct RectTransform {
		float Right, Left, Top, Down;
		Vec2 Pivot{ 0.5f, 0.5f };
		float Width{ 100 }, Height{ 100 };
		float Rotation{ 0.f };
	};
}