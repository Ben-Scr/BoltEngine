#include "pch.hpp"
#include "Random.hpp"
#include "Collections/Color.hpp"
#include <limits>

namespace Bolt {
	Color Random::NextColor() {
		return Color(NextFloat(0.f, 1.f), NextFloat(0.f, 1.f), NextFloat(0.f, 1.f));
	}
	bool Random::NextBool() {
		static thread_local std::bernoulli_distribution dist(0.5);
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen);
	}
	std::uint8_t Random::NextByte(std::uint8_t max) {
		static thread_local std::uniform_int_distribution<int> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	std::uint8_t Random::NextByte(std::uint8_t min, std::uint8_t max) {
		BT_ASSERT(min <= max, BoltErrorCode::InvalidArgument, "min can't be more than max");

		static thread_local std::uniform_int_distribution<int> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	double Random::NextDouble() {
		static thread_local std::uniform_real_distribution<double> dist(0.0, 1.f);
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen);
	}
	double Random::NextDouble(double max) {
		BT_ASSERT(max >= 0, BoltErrorCode::InvalidArgument, "max can't be less than 0");

		static thread_local std::uniform_real_distribution<double> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	double Random::NextDouble(double min, double max) {
		BT_ASSERT(min <= max, BoltErrorCode::InvalidArgument, "min can't be more than max");

		static thread_local std::uniform_real_distribution<double> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	float Random::NextFloat() {
		static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen);
	}
	float Random::NextFloat(float max) {
		BT_ASSERT(max >= 0, BoltErrorCode::InvalidArgument, "max can't be less than 0");

		static thread_local std::uniform_real_distribution<float> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(0.f, max));
	}
	float Random::NextFloat(float min, float max) {
		BT_ASSERT(min <= max, BoltErrorCode::InvalidArgument, "min can't be more than max");

		static thread_local std::uniform_real_distribution<float> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	int Random::NextInt() {
		static thread_local std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen);
	}
	int Random::NextInt(int max) {
		BT_ASSERT(max >= 0, BoltErrorCode::InvalidArgument, "max can't be less than 0");

		static thread_local std::uniform_int_distribution<int> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	int Random::NextInt(int min, int max) {
		BT_ASSERT(min <= max, BoltErrorCode::InvalidArgument, "min can't be more than max");

		static thread_local std::uniform_int_distribution<int> dist;
		std::scoped_lock lock(s_Mutex);
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}
}