#pragma once
#include <chrono>
#include <sstream>
#include <string>
#include <ostream>

namespace Bolt {
        class Timer {
            using Clock = std::chrono::high_resolution_clock;
            using TimePoint = std::chrono::time_point<Clock>;
            using Duration = std::chrono::duration<double>;

        public:
            static Timer Start() {
                return Timer(Clock::now());
            }

            void Continue() {
                if (m_paused) {
                    m_startTime = Clock::now();
                    m_paused = false;
                }
            }

            void Pause() {
                if (!m_paused) {
                    m_pausedDuration += Clock::now() - m_startTime;
                    m_paused = true;
                }
            }

            void Reset() {
                m_startTime = Clock::now();
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
            explicit Timer(TimePoint startTime)
                : m_startTime(startTime)
                , m_pausedDuration(Duration::zero())
                , m_paused(false) {
            }

            Duration GetTotalElapsed() const {
                if (m_paused) {
                    return m_pausedDuration;
                }
                else {
                    return m_pausedDuration + (Clock::now() - m_startTime);
                }
            }

            TimePoint m_startTime;
            Duration m_pausedDuration;
            bool m_paused;
        };


        inline std::ostream& operator<<(std::ostream& os, const Timer& timer) {
            return os << timer.ElapsedMilliseconds() << " ms";
        }
}