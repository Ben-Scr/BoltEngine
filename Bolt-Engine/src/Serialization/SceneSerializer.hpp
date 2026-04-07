#pragma once
#include "Core/Export.hpp"
#include <string>

namespace Bolt {

	class Scene;

	class BOLT_API SceneSerializer {
	public:
		static bool SaveToFile(Scene& scene, const std::string& path);
		static bool LoadFromFile(Scene& scene, const std::string& path);

	private:
		static void DeserializeEntity(Scene& scene, const std::string& entityJson);
	};

} // namespace Bolt
