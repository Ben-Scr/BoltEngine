#include "Bolt/btpch.hpp"
#include "Bolt/Core/Time.hpp"

namespace Bolt {
	float Time::s_TimeScale = 1.f;
	float Time::s_DeltaTime = 0.0f;
	float Time::s_SimulatedElapsedTime = 0.0f;
	int Time::s_FrameCount = 0;

	float Time::s_FixedDeltaTime = 1.0f / 50.f;
	float Time::s_TargetFPS = 144.f;
	float Time::s_MaxPossibleFPS = 0.f;
	float Time::s_UpdateDeltaTime = 1.0f / s_TargetFPS;

	std::chrono::steady_clock::duration Time::s_FrameDuration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
		std::chrono::duration<float>(1.0f / s_TargetFPS)
	);
	std::chrono::high_resolution_clock::time_point Time::s_StartTime = std::chrono::high_resolution_clock::now();

	void Time::SetTargetFramerate(float framerate) {
		s_TargetFPS = framerate;
		if (s_TargetFPS > 0)
			s_UpdateDeltaTime = 1.0f / s_TargetFPS;
		else
			s_UpdateDeltaTime = std::numeric_limits<float>::infinity();

		s_FrameDuration =
			std::chrono::duration_cast<std::chrono::steady_clock::duration>(
				std::chrono::duration<float>(1.0f / s_TargetFPS)
			);
	}

	void Time::SetTimeScale(float scale) {
		s_TimeScale = scale;
	}

	void Time::SetFixedDeltaTime(float step) {
		step = Clamp(step, 0.f, 1.f);
		s_FixedDeltaTime = step;
	}

	float Time::GetElapsedTime() {
		std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now() - s_StartTime;
		return elapsed.count();
	}

	float Time::GetSimulatedElapsedTime() { return s_SimulatedElapsedTime; }

	void Time::Update(float deltaTime) {
		Time::s_DeltaTime = deltaTime;
		s_SimulatedElapsedTime += Time::s_DeltaTime * s_TimeScale;
		s_FrameCount++;
	}
}