#pragma once
#include <chrono>

namespace Bolt {
	class Application;

	class Time {
	public:
		static float GetDeltaTime() { return s_DeltaTime * s_TimeScale; }
		static float GetDeltaTimeUnscaled() { return s_DeltaTime; }
		static void SetTargetFramerate(float fps);
		static float GetTargetFramerate() { return s_TargetFPS; }

		static float GetFixedDeltaTime() { return s_FixedDeltaTime * s_TimeScale; }
		static void SetFixedDeltaTime(float step);

		static float UnscaledFixedDeltaTime() { return s_FixedDeltaTime; }
		static float GetMaxPossibleFPS() { return s_MaxPossibleFPS; }

		static float GetTimeScale() { return s_TimeScale; }
		static void SetTimeScale(float scale);

		// Info: Realtime elapsed time
		static float GetElapsedTime();
		// Info: Elapsed time based on timescale
		static float GetSimulatedElapsedTime();

	private:
		static void Update(float deltaTime);
		static float s_DeltaTime;
		static float s_TargetFPS;
		static float s_TimeScale;
		static float s_UpdateDeltaTime;
		static float s_FixedDeltaTime;
		static float s_SimulatedElapsedTime;
		static int s_FrameCount;
		static float s_MaxPossibleFPS;

		static std::chrono::steady_clock::duration s_FrameDuration;
		static std::chrono::high_resolution_clock::time_point s_StartTime;

		friend class Application;
	};
}