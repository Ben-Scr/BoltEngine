#include "../pch.hpp"
#include "../Scene/SceneDefinition.hpp"
#include "../Scene/Scene.hpp"


namespace Bolt {
	std::unique_ptr<Scene> SceneDefinition::Instantiate() const {
		auto scene = std::unique_ptr<Scene>(
			new Scene(m_Name, this, m_IsPersistent)
		);

		for (const auto& factory : m_SystemFactories) {
			try {
				auto system = factory();
				if (system) {
					scene->m_Systems.push_back(std::move(system));
				}
				else {
					Logger::Error("Failed to create system for scene with name " + m_Name);
				}
			}
			catch (const std::exception& e) {
				Logger::Error("Exception creating system for scene with name '" +
					m_Name + "': " + e.what());
			}
		}


		for (const auto& callback : m_InitializeCallbacks) {
			try {
				callback(*scene);
			}
			catch (const std::exception& e) {
				Logger::Error("Exception in initialize callback for scene with name '" +
					m_Name + "': " + e.what());
			}
		}

		scene->AwakeSystems();
		scene->StartSystems();

		return scene;
	}
}