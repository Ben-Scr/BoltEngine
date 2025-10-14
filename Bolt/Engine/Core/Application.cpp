#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"
#include "Time.hpp"
#include "Input.hpp"


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

		m_PhysicsSystem.value().FixedUpdate(0.01f);
		SceneManager::UpdateScenes();
		m_Renderer2D.value().BeginFrame();
		Input::Update();
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
		m_PhysicsSystem.emplace(PhysicsSystem());
	}
}