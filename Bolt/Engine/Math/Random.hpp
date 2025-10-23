#pragma once
#include <random>
#include <type_traits>
#include <cstdint>

namespace Bolt {

    class Random {
    public:
        Random() : m_Gen(std::random_device{}()) {}
        explicit Random(uint32_t seed) : m_Gen(seed) {}

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        T Next() {
            static thread_local std::uniform_real_distribution<T> distr((T)0, (T)1);
            return distr(m_Gen);                 // [0,1)
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        T Next(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_int_distribution<T> distr;
            return distr(m_Gen, typename decltype(distr)::param_type(from, to)); // inkl. beider Grenzen
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        T Next(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_real_distribution<T> distr;
            return distr(m_Gen, typename decltype(distr)::param_type(from, to)); // [from,to)
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        static T Range() {
            static thread_local std::uniform_real_distribution<T> distr((T)0, (T)1);
            return distr(s_Gen);                 // [0,1)
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        static T Range(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_int_distribution<T> distr;
            return distr(s_Gen, typename decltype(distr)::param_type(from, to)); // inkl. beider Grenzen
        }

        template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        static T Range(T from, T to) {
            if (from > to) std::swap(from, to);
            static thread_local std::uniform_real_distribution<T> distr;
            return distr(s_Gen, typename decltype(distr)::param_type(from, to)); // [from,to)
        }

        static void Seed(uint32_t seed) { s_Gen.seed(seed); }

    private:
        std::mt19937 m_Gen;
        inline static std::mt19937 s_Gen{ std::random_device{}() }; // C++17 inline variable
    };

}
