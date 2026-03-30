#include <Bolt.hpp>

#include "Core/Application.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include <Systems/ImGuiEditorSystem.hpp>
#include <Core/Version.hpp>

using namespace Bolt;

class EditorApplication : public Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		config.WindowProps = WindowProps(1600, 900, "Bolt Editor " + std::string(BT_VERSION), true, true, true);
		config.EnableAudio = false;
		config.EnableGizmoRenderer = false;
		config.EnableGuiRenderer = false;
		config.EnablePhysics2D = true;
		config.SetWindowIcon = false;
		return config;
	}

	void ConfigureScenes() override {
		SceneDefinition& editorScene = GetSceneManager()->RegisterScene("SampleScene");
		editorScene.AddSystem<ImGuiEditorSystem>();
		editorScene.SetAsStartupScene();
	}

	void Start() override {
		EntityHelper::CreateCamera2DEntity();
	}

	void Update() override {
		if (GetInput().GetKeyDown(KeyCode::E))
			Logger::Message("Hello World! " + StringHelper::ToString(Random::NextInt(0, 100)));
	}
	void FixedUpdate() override {}
	void OnPaused() override {}
	void OnQuit() override {}
};

Bolt::Application* Bolt::CreateApplication() {
	return new EditorApplication();
}