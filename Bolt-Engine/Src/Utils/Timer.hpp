#pragma once
#include <chrono>
#include <string>
#include <iostream>

namespace Bolt {
	class Timer {
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		using Duration = std::chrono::duration<double>;

	public:
		Timer() : m_StartTime(Clock::now())
			, m_pausedDuration(Duration::zero())
			, m_paused(false) {

		}

		void Continue() {
			if (m_paused) {
				m_StartTime = Clock::now();
				m_paused = false;
			}
		}

		void Pause() {
			if (!m_paused) {
				m_pausedDuration += Clock::now() - m_StartTime;
				m_paused = true;
			}
		}

		void Reset() {
			m_StartTime = Clock::now();
			m_pausedDuration = Duration::zero();
			m_paused = false;
		}

		float ElapsedSeconds() const {
			auto totalElapsed = GetTotalElapsed();
			return std::chrono::duration_cast<std::chrono::seconds>(totalElapsed).count();
		}

		float ElapsedMilliseconds() const {
			auto totalElapsed = GetTotalElapsed();
			return static_cast<float>(std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(totalElapsed).count());
		}

		uint64_t ElapsedMicroseconds() const {
			auto totalElapsed = GetTotalElapsed();
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(totalElapsed).count());
		}

		bool IsPaused() const {
			return m_paused;
		}

	private:
		Duration GetTotalElapsed() const {
			if (m_paused) {
				return m_pausedDuration;
			}
			else {
				return m_pausedDuration + (Clock::now() - m_StartTime);
			}
		}

		TimePoint m_StartTime;
		Duration m_pausedDuration;
		bool m_paused;
	};


	inline std::ostream& operator<<(std::ostream& os, const Timer& timer) {
		return os << timer.ElapsedMilliseconds() << " ms";
	}

	class ScopedTimer
	{
	public:
		ScopedTimer(const std::string& name, const std::string& description)
			: m_Name(name), m_Description(description) {
		}
		~ScopedTimer()
		{
			float time = m_Timer.ElapsedMilliseconds();
			std::ostringstream oss;
			oss << m_Description << " - " << time << "ms";
			Logger::Message(m_Name, oss.str());
		}
	private:
		std::string m_Name;
		std::string m_Description;
		Timer m_Timer;
	};
}