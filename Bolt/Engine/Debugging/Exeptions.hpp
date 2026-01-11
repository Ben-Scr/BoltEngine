#pragma once
#include <stdexcept>

namespace Bolt {
    class SceneExeption : public std::runtime_error {
    public:
        explicit SceneExeption(const std::string& msg): std::runtime_error(msg) {}
        explicit SceneExeption() : std::runtime_error("") {}
    };
}