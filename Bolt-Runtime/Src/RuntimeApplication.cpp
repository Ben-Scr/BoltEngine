#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include <Scene/EntityHelper.hpp>

#include <Core/Version.hpp>

using namespace Bolt;


class RuntimeApplication : public Bolt::Application {
public:
	ApplicationConfig GetConfiguration() const override {
		ApplicationConfig config;
		config.WindowSpecification = WindowSpecification(800, 800, "Bolt Runtime Application " + std::string(BT_VERSION), true, true, false);
		config.EnableAudio = false;
		return config;
	}

	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("SampleScene");
		def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~RuntimeApplication() override = default;

	void Start() override {
		BT_INFO("Start");
		EntityHelper::CreateCamera2DEntity();
	}
	void Update() override {
		auto* app = Application::GetInstance();
		auto input = app->GetInput();
	}
	void FixedUpdate() override {

	}
	void OnPaused() override {

	}
	void OnQuit() override {
		BT_INFO("Quit");
	}
};



Bolt::Application* Bolt::CreateApplication() {
	return new RuntimeApplication();
}