#pragma once
#include <filesystem>
#include <string>
#include <sstream>

namespace Bolt {
    class Path {
    public:
        template <typename... Args>
        static std::string Combine(Args&&... args) {
            std::filesystem::path combined;
            (combined.append(std::filesystem::path(std::forward<Args>(args))), ...);
            return combined.make_preferred().string();
        }

        static std::string Current() {
            return std::filesystem::current_path().string();
        }
    };
}