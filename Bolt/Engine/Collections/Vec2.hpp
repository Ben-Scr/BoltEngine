#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace glm {
	inline bool operator<(const glm::vec2& a, const glm::vec2& b) {
		if (a.x < b.x) return true;
		if (a.x > b.x) return false;
		return a.y < b.y;
	}
}

namespace Bolt {
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec2Int = glm::ivec2;
	inline std::string ToString(const Vec2& v) {
		return "{ " + std::to_string(v.x) + ", " + std::to_string(v.y) + " }";
	}
}