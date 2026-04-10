#pragma once
#include "Core/Export.hpp"
#include "Scene/EntityHandle.hpp"
#include <string>

namespace Bolt {

	class Scene;

	class Entity;

	class BOLT_API SceneSerializer {
	public:
		static bool SaveToFile(Scene& scene, const std::string& path);
		static bool LoadFromFile(Scene& scene, const std::string& path);

		// Prefab support: save/load single entities
		static bool SaveEntityToFile(Scene& scene, EntityHandle entity, const std::string& path);
		static EntityHandle LoadEntityFromFile(Scene& scene, const std::string& path);

	private:
		static void DeserializeEntity(Scene& scene, const std::string& entityJson);
	};

} // namespace Bolt
