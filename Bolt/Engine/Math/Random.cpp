#include "../pch.hpp"
#include "Random.hpp"
#include "../Collections/Color.hpp"

namespace Bolt {
	Color Random::NextColor() {
		return Color(Range(0.f, 1.f), Range(0.f, 1.f), Range(0.f, 1.f));
	}
	bool Random::NextBool() {
		return Range<std::uint8_t>(0, 1) == 0;
	}
}