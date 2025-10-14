#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"
#include "Time.hpp"
#include "Input.hpp"


namespace Bolt {
	float Application::s_TargetFramerate = 14400;

	void Application::Run()
	{
		Initialize();

		// Main loop
		while (!m_Window.value().ShouldClose()) {
			using Clock = std::chrono::high_resolution_clock;
			using Dur = Clock::duration;

			float fixedUpdateAccumulator = 0.0f;


			Dur TARGET_FRAME_TIME = std::chrono::duration_cast<Dur>(std::chrono::duration<double>(1.0 / s_TargetFramerate));
			static auto lastTime = Clock::now();
			auto const nextFrameTime = lastTime + TARGET_FRAME_TIME;

			auto now = Clock::now();
			if (now + std::chrono::milliseconds(10) < nextFrameTime) {
				std::this_thread::sleep_until(nextFrameTime - std::chrono::milliseconds(10));
			}
			while (Clock::now() < nextFrameTime) {
				_mm_pause();
			}
			auto frameStart = Clock::now();

			float deltaTime = std::chrono::duration<float>(frameStart - lastTime).count();
			Time::Update(deltaTime);

			fixedUpdateAccumulator += Time::GetDeltaTime();
			while (fixedUpdateAccumulator >= Time::s_FixedDeltaTime) {
				{
					try {
						BeginFixedFrame();
					}
					catch (std::runtime_error e) {
						Logger::Error(e.what());
					}
				}

				fixedUpdateAccumulator -= Time::s_FixedDeltaTime;
			}

			BeginFrame();
			EndFrame();

			lastTime = frameStart;
		}
	}

	void Application::BeginFrame() {
		CoreInput();
		m_PhysicsSystem.value().FixedUpdate(0.01f);
		SceneManager::UpdateScenes();
		m_Renderer2D.value().BeginFrame();
		Time::Update(0.01f);
	}

	void Application::BeginFixedFrame() {

	}

	void Application::EndFrame() {
		m_Renderer2D.value().EndFrame();
		m_Window.value().SwapBuffers();
		Input::Update();
		glfwPollEvents();
	}

	void Application::CoreInput() {
		if (Input::GetKeyDown(KeyCode::Esc)) {
			m_Window.value().MinimizeWindow();
		}
		if (Input::GetKeyDown(KeyCode::F11)) {
			m_Window.value().MaximizeWindow(true);
		}
		if (Input::GetKeyDown(KeyCode::F12)) {
			m_Window.value().SetVsync(!m_Window.value().IsVsync());
		}

		std::string fps = std::to_string(1.f / Time::GetDeltaTimeUnscaled()) + " FPS";
		m_Window.value().SetTitle(fps);
	}

	void Application::Initialize() {

		GLFWWindowProperties windowProps;

		m_Window.emplace(Window(windowProps));
		m_Window.value().SetVsync(true);

		m_Renderer2D.emplace(Renderer2D());

		GLInitProperties glInitProps = GLInitProperties(Color{ 0.3f, 0.3f, 0.3f }, true, GLCullingModes::GLBack);
		m_Renderer2D.value().Initialize(glInitProps);

		SceneManager::Initialize();
		m_PhysicsSystem.emplace(PhysicsSystem());
	}
}