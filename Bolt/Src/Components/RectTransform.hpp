#pragma once
#include "../Collections/Vec2.hpp"

namespace Bolt {
	struct RectTransform {
		float Right{ 0.0f }, Left{ 0.0f }, Top{ 0.0f }, Bottom{ 0.0f };
		Vec2 Pivot{ 0.0f, 0.0f };
		float Width{ 100 }, Height{ 100 };
		float Rotation{ 0.f };
	};
}