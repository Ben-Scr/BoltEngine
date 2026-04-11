#include <Bolt.hpp>

#include "Core/Application.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include <Systems/ImGuiEditorSystem.hpp>
#include <Systems/ImGuiDebugSystem.hpp>
#include <Systems/GizmosDebugSystem.hpp>
#include <Systems/ParticleUpdateSystem.hpp>
#include <Scripting/ScriptSystem.hpp>
#include <Project/ProjectManager.hpp>
#include <Project/BoltProject.hpp>
#include <Serialization/SceneSerializer.hpp>
#include <Systems/AudioUpdateSystem.hpp>
#include <Core/Version.hpp>

using namespace Bolt;

// Parsed from command line before Application starts
static std::string s_ProjectPath;

class EditorApplication : public Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		std::string title = "Bolt Editor " + std::string(BT_VERSION);
		if (ProjectManager::HasProject())
			title += " - " + ProjectManager::GetCurrentProject()->Name;
		config.WindowSpecification = WindowSpecification(0, 0, title, true, true, true);
		config.EnableAudio = true;
		config.EnableGizmoRenderer = true;
		config.EnableGuiRenderer = false;
		config.EnablePhysics2D = true;
		config.SetWindowIcon = true;
		SetRunInBackground(true);
		return config;
	}

	void ConfigureScenes() override {
		SceneDefinition& editorScene = GetSceneManager()->RegisterScene("SampleScene");
		editorScene.AddSystem<ImGuiEditorSystem>();
		editorScene.AddSystem<ImGuiDebugSystem>();
		editorScene.AddSystem<GizmosDebugSystem>();
		editorScene.AddSystem<ParticleUpdateSystem>();
		editorScene.AddSystem<ScriptSystem>();
		editorScene.AddSystem<AudioUpdateSystem>();
		editorScene.SetAsStartupScene();
	}

	void Start() override {}
	void Update() override {}
	void FixedUpdate() override {}
	void OnPaused() override {}

	void OnQuit() override {
		BoltProject* project = ProjectManager::GetCurrentProject();
		if (project) {
			auto* sceneManager = GetSceneManager();
			if (sceneManager) {
				Scene* active = sceneManager->GetActiveScene();
				if (active) {
					std::string scenePath = project->GetSceneFilePath(active->GetName());
					SceneSerializer::SaveToFile(*active, scenePath);
					project->LastOpenedScene = active->GetName();
					project->Save();
					BT_INFO_TAG("Editor", "Auto-saved scene on quit: {}", scenePath);
				}
			}
		}
	}
};

Bolt::Application* Bolt::CreateApplication() {
	// Parse --project="path" from command line
	int argc = __argc;
	char** argv = __argv;

	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg.rfind("--project=", 0) == 0) {
			s_ProjectPath = arg.substr(10);
			// Remove surrounding quotes if present
			if (s_ProjectPath.size() >= 2 && s_ProjectPath.front() == '"' && s_ProjectPath.back() == '"')
				s_ProjectPath = s_ProjectPath.substr(1, s_ProjectPath.size() - 2);
		}
	}

	if (!s_ProjectPath.empty() && BoltProject::Validate(s_ProjectPath)) {
		auto project = std::make_unique<BoltProject>(BoltProject::Load(s_ProjectPath));
		BT_CORE_INFO_TAG("Editor", "Opening project: {} ({})", project->Name, project->RootDirectory);
		ProjectManager::SetCurrentProject(std::move(project));
	}

	if (!ProjectManager::HasProject()) {
		BT_CORE_WARN_TAG("Editor", "No project specified. Use: Bolt-Editor.exe --project=\"path/to/project\"");
		BT_CORE_WARN_TAG("Editor", "Starting with default fallback paths.");
	}

	return new EditorApplication();
}
