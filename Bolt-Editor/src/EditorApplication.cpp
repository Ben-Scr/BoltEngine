#include <Bolt.hpp>

#include "Core/Application.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include <Systems/ImGuiEditorSystem.hpp>
#include <Systems/ImGuiDebugSystem.hpp>
#include <Systems/GizmosDebugSystem.hpp>
#include <Core/Version.hpp>

using namespace Bolt;

class EditorApplication : public Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		config.WindowSpecification = WindowSpecification(0,0, "Bolt Editor " + std::string(BT_VERSION), true, true, true);
		config.EnableAudio = false;
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
		editorScene.SetAsStartupScene();
	}

	void Start() override {
		EntityHelper::CreateCamera2DEntity();

		// Test Scenario Case: The engine should be able to handle this without crashing.
		//SceneDefinition& editorScene = GetSceneManager()->RegisterScene("SampleScene");
	}

	void Update() override {
		if (GetInput().GetKey(KeyCode::I))
			BT_INFO(std::to_string(1.0f / GetTime().GetDeltaTime()) + " FPS");

		if (GetInput().GetKeyDown(KeyCode::E))
			BT_INFO("Hello World! " + StringHelper::ToString(Random::NextInt(0, 100)));
	}

	void FixedUpdate() override {}
	void OnPaused() override {}
	void OnQuit() override {}
};

Bolt::Application* Bolt::CreateApplication() {
	return new EditorApplication();
}