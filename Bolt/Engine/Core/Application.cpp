#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"


namespace Bolt {
	void Application::Run()
	{
		Initialize();

		// Main loop
		while (!m_Window.value().ShouldClose()) {
			BeginFrame();
			EndFrame();
		}
	}

	void Application::BeginFrame() {
	//	static Texture2D tex("Assets/Textures/Square.png",
		//	Filter::Trilinear, Wrap::Clamp, Wrap::Clamp,
		//	true, true, true);

		SceneManager::UpdateScenes();
		m_Renderer2D.value().BeginFrame();
		//m_Renderer2D->DrawSprite({ 0.0f, 0.0f }, { 128.0f, 128.0f },
		//	0.0f, Color::White(), &tex);
	}

	void Application::EndFrame() {
		m_Renderer2D.value().EndFrame();
		m_Window.value().SwapBuffers();
		glfwPollEvents();
	}

	void Application::Initialize() {

		GLFWWindowProperties windowProps;

		m_Window.emplace(Window(windowProps));
		m_Window.value().SetVsync(true);

		m_Renderer2D.emplace(Renderer2D());

		GLInitProperties glInitProps = GLInitProperties(Color{0.3f, 0.3f, 0.3f}, true, GLCullingModes::GLBack);
		m_Renderer2D.value().Initialize(glInitProps);
		SceneManager::Initialize();
	}
}