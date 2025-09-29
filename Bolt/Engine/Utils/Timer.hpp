#pragma once
#include <chrono>
#include <string>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

namespace Bolt {
        class Timer {
        public:
            // Static factory method to create and start a new timer
            static Timer Start() {
                return Timer(Clock::now());
            }

            // Continue timing after pause (resumes from where it was paused)
            void Continue() {
                if (m_paused) {
                    m_startTime = Clock::now();
                    m_paused = false;
                    // m_pausedDuration keeps the accumulated time from before
                }
            }

            // Pause the timer (accumulates elapsed time)
            void Pause() {
                if (!m_paused) {
                    m_pausedDuration += Clock::now() - m_startTime;
                    m_paused = true;
                }
            }

            // Reset timer to zero and restart
            void Reset() {
                m_startTime = Clock::now();
                m_pausedDuration = Duration::zero();
                m_paused = false;
            }

            // Get elapsed time in seconds (as integer)
            float ElapsedSeconds() const {
                auto totalElapsed = GetTotalElapsed();
                return std::chrono::duration_cast<std::chrono::seconds>(totalElapsed).count();
            }

            // Get elapsed time in milliseconds (as float for precision)
            float ElapsedMilliseconds() const {
                auto totalElapsed = GetTotalElapsed();
                return static_cast<float>(std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(totalElapsed).count());
            }

            // Additional utility: Get elapsed time in microseconds
            uint64_t ElapsedMicroseconds() const {
                auto totalElapsed = GetTotalElapsed();
                return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(totalElapsed).count());
            }

            std::string ToString() { return std::to_string(ElapsedMilliseconds()) + " ms"; }

            // Check if timer is currently paused
            bool IsPaused() const {
                return m_paused;
            }

        private:
            // Private constructor - use Start() to create instances
            explicit Timer(TimePoint startTime)
                : m_startTime(startTime)
                , m_pausedDuration(Duration::zero())
                , m_paused(false) {
            }

            // Calculate total elapsed time including paused periods
            Duration GetTotalElapsed() const {
                if (m_paused) {
                    return m_pausedDuration;
                }
                else {
                    return m_pausedDuration + (Clock::now() - m_startTime);
                }
            }

            TimePoint m_startTime;          // When timer was started/last resumed
            Duration m_pausedDuration;      // Accumulated time while running
            bool m_paused;                  // Current pause state
        };
}