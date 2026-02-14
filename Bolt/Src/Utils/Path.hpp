#pragma once
#include <filesystem>
#include <string>

namespace Bolt {
	class Path {
	public:
		template <typename... Args>
        static std::string Combine(Args&&... args) {
            std::ostringstream oss;
            bool first = true;

            auto append = [&](auto&& part) {
                if (!first) oss << '/';
                first = false;
                oss << std::forward<decltype(part)>(part);
                };

            (append(std::forward<Args>(args)), ...);
            return oss.str();
        }

        static std::string Current() {
            return std::filesystem::current_path().string();
        }
	};
}