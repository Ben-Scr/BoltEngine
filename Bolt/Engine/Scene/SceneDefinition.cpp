#include "../pch.hpp"
#include "../Scene/SceneDefinition.hpp"
#include "../Scene/Scene.hpp"


namespace Bolt {
	std::unique_ptr<Scene> SceneDefinition::Instantiate() const {
		auto scene = std::unique_ptr<Scene>(
			new Scene(m_name, this, m_isPersistent)
		);

		for (const auto& factory : m_systemFactories) {
			try {
				auto system = factory();
				if (system) {
					scene->m_Systems.push_back(std::move(system));
				}
				else {
					Logger::Error("Failed to create system for scene with name " + m_name);
				}
			}
			catch (const std::exception& e) {
				Logger::Error("Exception creating system for scene with name '" +
					m_name + "': " + e.what());
			}
		}


		for (const auto& callback : m_initializeCallbacks) {
			try {
				callback(*scene);
			}
			catch (const std::exception& e) {
				Logger::Error("Exception in initialize callback for scene with name '" +
					m_name + "': " + e.what());
			}
		}

		scene->AwakeSystems();
		scene->StartSystems();

		return scene;
	}
}