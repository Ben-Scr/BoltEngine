#pragma once
#include "Shader.hpp"
#include <memory>

// Todo
namespace Bolt {
	class ShaderManager {
	public:
		std::unique_ptr<Shader> LoadShader(const std::string_view& vert, const std::string_view& frag) {

		}

	};
}