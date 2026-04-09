#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include <Systems/ParticleUpdateSystem.hpp>
#include <Scripting/ScriptSystem.hpp>
#include <Scene/EntityHelper.hpp>
#include <Serialization/SceneSerializer.hpp>
#include <Serialization/Path.hpp>
#include <Serialization/File.hpp>
#include <Project/ProjectManager.hpp>
#include <Project/BoltProject.hpp>

#include <Core/Version.hpp>
#include <Core/Window.hpp>
#include <filesystem>

using namespace Bolt;


class RuntimeApplication : public Bolt::Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		BoltProject* project = ProjectManager::GetCurrentProject();
		std::string title = project
			? project->Name + " - Bolt Runtime"
			: "Bolt Runtime Application " + std::string(BT_VERSION);

		if (project) {
			config.WindowSpecification = WindowSpecification(
				project->BuildWidth, project->BuildHeight, title,
				project->BuildResizable, true, project->BuildFullscreen);
		} else {
			config.WindowSpecification = WindowSpecification(800, 800, title, true, true, false);
		}
		config.EnableAudio = false;
		config.EnablePhysics2D = true;
		return config;
	}

	void ConfigureScenes() override {
		BoltProject* project = ProjectManager::GetCurrentProject();
		std::string sceneName = "SampleScene";
		if (project) {
			if (!project->StartupScene.empty())
				sceneName = project->StartupScene;
			else if (!project->LastOpenedScene.empty())
				sceneName = project->LastOpenedScene;
		}

		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene(sceneName);
		def.AddSystem<ScriptSystem>();
		def.AddSystem<ParticleUpdateSystem>();
		def.SetAsStartupScene();
	}

	~RuntimeApplication() override = default;

	void Start() override {
		BoltProject* project = ProjectManager::GetCurrentProject();
		if (project) {
			Scene* active = GetSceneManager()->GetActiveScene();
			if (active) {
				std::string scenePath = project->GetSceneFilePath(active->GetName());
				if (File::Exists(scenePath)) {
					SceneSerializer::LoadFromFile(*active, scenePath);
				}
			}
		} else {
			EntityHelper::CreateCamera2DEntity();
		}
	}

	void Update() override {}
	void FixedUpdate() override {}
	void OnPaused() override {}

	void OnQuit() override {
		BT_INFO("Quit");
	}
};


Bolt::Application* Bolt::CreateApplication() {
	// Auto-detect project: look for bolt-project.json next to the executable
	std::string exeDir = Path::ExecutableDir();
	if (BoltProject::Validate(exeDir)) {
		auto project = std::make_unique<BoltProject>(BoltProject::Load(exeDir));
		BT_CORE_INFO_TAG("Runtime", "Loaded project: {} ({})", project->Name, project->RootDirectory);
		ProjectManager::SetCurrentProject(std::move(project));
	}

	return new RuntimeApplication();
}
