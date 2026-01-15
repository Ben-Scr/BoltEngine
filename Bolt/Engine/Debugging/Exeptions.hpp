#pragma once
#include <stdexcept>

namespace Bolt {
	class SceneExeption : public std::runtime_error {
	public:
		explicit SceneExeption(const std::string& msg) : std::runtime_error("SceneExeption:" + msg) {}
	};
	class InvalidArgumentExeption : public std::runtime_error {
	public:
		explicit InvalidArgumentExeption(const std::string& msg) : std::runtime_error("InvalidArgumentExeption: " + msg) {}
	};
	class NullReferenceArgumentExeption : public std::runtime_error {
	public:
		explicit NullReferenceArgumentExeption(const std::string& msg) : std::runtime_error("NullReferenceArgumentExeption: " + msg) {}
	};
}