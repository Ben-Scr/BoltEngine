#include <Bolt.hpp>

#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Time.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/ISystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"

using namespace Bolt;

namespace {
	class EditorUiSystem : public ISystem {
	public:
		void OnGui(Scene& scene) override {
			// ImGui ist vorübergehend vollständig deaktiviert.
			(void)scene;
		}
	};
}

class BoltEditorApplication : public Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		config.WindowProps = WindowProps(1600, 900, "Bolt Editor", true, true, false);
		config.EnableImGui = false;
		config.EnableAudio = false;
		config.SetWindowIcon = false;
		return config;
	}

	void ConfigureScenes() override {
		SceneDefinition& editorScene = GetSceneManager()->RegisterScene("EditorScene");
		editorScene.AddSystem<EditorUiSystem>();
		editorScene.SetAsStartupScene();
	}

	void Start() override {
		EntityHelper::CreateCamera2DEntity();
	}

	void Update() override {}
	void FixedUpdate() override {}
	void OnPaused() override {}
	void OnQuit() override {}
};

Bolt::Application* Bolt::CreateApplication() {
	return new BoltEditorApplication();
}