#pragma once
#include <string>
#include <ostream>

namespace Bolt {
	class StringHelper {
	public:
		static std::string WrapWith(std::string_view s, char mark) {
			std::string out;
			out.reserve(s.size() + 2);
			out.push_back(mark);
			out.append(s);
			out.push_back(mark);
			return out;
		}
		static std::string WrapWith(std::string_view s, std::string_view mark) {
			std::string result;
			result.reserve(s.size() + (mark.size() * 2));
			result += mark;
			result += s;
			result += mark;
			return result;
		}

		template <typename... Args>
		static std::string ToString(Args&&... args) {
			std::ostringstream oss;
			((oss << std::forward<Args>(args)), ...);
			return oss.str();
		}
	};
}