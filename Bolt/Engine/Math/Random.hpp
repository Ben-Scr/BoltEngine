#pragma once
#include <random>
#include <type_traits>
#include <cstdint>

namespace Bolt {
    struct Color;

    class Random {
    public:
        Random() : m_Gen(std::random_device{}()) {}
        explicit Random(uint32_t seed) : m_Gen(seed) {}

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        T Next() {
            static thread_local std::uniform_real_distribution<T> dist((T)0, (T)1);
            return dist(m_Gen);
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        T Next(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_int_distribution<T> dist;
            return dist(m_Gen, typename decltype(dist)::param_type(from, to));
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        T Next(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_real_distribution<T> dist;
            return dist(m_Gen, typename decltype(dist)::param_type(from, to));
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        static T Range() {
            static thread_local std::uniform_real_distribution<T> dist((T)0, (T)1);
            return dist(s_Gen);
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        static T Range(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_int_distribution<T> dist;
            return dist(s_Gen, typename decltype(dist)::param_type(from, to));
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        static T Range(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_real_distribution<T> dist;
            return dist(s_Gen, typename decltype(dist)::param_type(from, to));
        }

        static void SetSeed(uint32_t seed) { s_Gen.seed(seed); }

        static bool NextBool();
        static Color NextColor();

    private:
        std::mt19937 m_Gen;
        inline static std::mt19937 s_Gen{ std::random_device{}() };
    };

}
