#include "../pch.hpp"
#include "../Scene/SceneDefinition.hpp"
#include "../Scene/Scene.hpp"


namespace Bolt {
	std::shared_ptr<Scene> SceneDefinition::Instantiate() const {
		std::shared_ptr<Scene> scene(new Scene(m_Name, this, m_IsPersistent));

		for (const auto& factory : m_SystemFactories) {
			try {
				auto system = factory();

				if (system) {
					system->SetScene(scene);
					scene->m_Systems.push_back(std::move(system));
				}
				else {
					throw "";
				}
			}
			catch (...) {
				throw SceneExeption("Failed creating system for scene with name '" +
					m_Name + "'");
			}
		}

		for (const auto& callback : m_InitializeCallbacks) {
			try {
				callback(*scene);
			}
			catch (const std::exception& e) {
				throw SceneExeption("Exception in initialize callback for scene with name '" +
					m_Name + "': " + e.what());
			}
			catch (...) {
				throw SceneExeption("Unknown Exception in initialize callback for scene with name '" +
					m_Name + "'");
			}
		}

		scene->AwakeSystems();
		scene->StartSystems();

		return scene;
	}
}