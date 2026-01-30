#pragma once
#include <cmath>

namespace Bolt {
	struct Quaternion {
		float x, y, z, w;
		Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
			: x(x), y(y), z(z), w(w) {
		}
		void Normalize() {
			const float length = std::sqrt(x * x + y * y + z * z + w * w);
			if (length > 0.0f) {
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}
		}
	};
}