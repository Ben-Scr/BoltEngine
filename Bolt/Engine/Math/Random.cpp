#include "../pch.hpp"
#include "Random.hpp"
#include "../Collections/Color.hpp"

namespace Bolt {
	Color Random::NextColor() {
		return Color(NextFloat(0.f, 1.f), NextFloat(0.f, 1.f), NextFloat(0.f, 1.f));
	}
	bool Random::NextBool() {
		static thread_local std::bernoulli_distribution dist(0.5);
		return dist(s_Gen);
	}
	std::uint8_t Random::NextByte(std::uint8_t max) {
		static thread_local std::uniform_int_distribution<int> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	std::uint8_t Random::NextByte(std::uint8_t min, std::uint8_t max) {
		if (min > max) throw InvalidArgumentExeption("min can't be more than max");

		static thread_local std::uniform_int_distribution<int> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	double Random::NextDouble() {
		static thread_local std::uniform_real_distribution<double> dist(0.0, 1.f);
		return dist(s_Gen);
	}
	double Random::NextDouble(double max) {
		if (max < 0) throw InvalidArgumentExeption("max can't be less than 0");

		static thread_local std::uniform_real_distribution<double> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	double Random::NextDouble(double min, double max) {
		if (min > max) throw InvalidArgumentExeption("min can't be more than max");

		static thread_local std::uniform_real_distribution<double> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	float Random::NextFloat() {
		static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
		return dist(s_Gen);
	}
	float Random::NextFloat(float max) {
		if (max < 0.f) throw InvalidArgumentExeption("max can't be less than 0");

		static thread_local std::uniform_real_distribution<float> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(0.f, max));
	}
	float Random::NextFloat(float min, float max) {
		if (min > max) throw InvalidArgumentExeption("min can't be more than max");

		static thread_local std::uniform_real_distribution<float> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}

	int Random::NextInt() {
		static thread_local std::uniform_int_distribution<int> dist(0, 1);
		return dist(s_Gen);
	}
	int Random::NextInt(int max) {
		if (max < 0.f) throw InvalidArgumentExeption("max can't be less than 0");

		static thread_local std::uniform_int_distribution<int> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(0, max));
	}
	int Random::NextInt(int min, int max) {
		if (min > max) throw InvalidArgumentExeption("min can't be more than max");

		static thread_local std::uniform_int_distribution<int> dist;
		return dist(s_Gen, typename decltype(dist)::param_type(min, max));
	}
}