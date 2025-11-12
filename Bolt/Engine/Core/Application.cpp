#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Graphics/TextureManager.hpp"
#include "../Graphics/OpenGL.hpp"
#include "Time.hpp"
#include "Input.hpp"


namespace Bolt {
	float Application::s_TargetFramerate = 144;
	Application* Application::s_Instance = nullptr;

	void Application::Run()
	{
		Initialize();

		while (!m_Window.value().ShouldClose()) {
			m_LoopedThisFrame = false;
			using Clock = std::chrono::high_resolution_clock;
			using Duration = Clock::duration;

			static float fixedUpdateAccumulator = 0.0f;


			Duration TARGET_FRAME_TIME = std::chrono::duration_cast<Duration>(std::chrono::duration<double>(1.0 / s_TargetFramerate));
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
				try {

					BeginFixedFrame();
				}
				catch (std::runtime_error e) {
					Logger::Error(e.what());
				}


				fixedUpdateAccumulator -= Time::s_FixedDeltaTime;
			}

			if (m_LoopedThisFrame)
			{
				lastTime = frameStart;
				continue;
			}

			BeginFrame();
			EndFrame();
			glfwPollEvents();

			lastTime = frameStart;
			m_LoopedThisFrame = true;
		}
	}


	void Application::BeginFrame() {
		CoreInput();
		SceneManager::UpdateScenes();
		m_Renderer2D.value().BeginFrame();
		m_GizmoRenderer.value().BeginFrame();
	}

	void Application::BeginFixedFrame() {
		SceneManager::FixedUpdateScenes();
		m_PhysicsSystem.value().FixedUpdate(Time::GetFixedDeltaTime());
	}

	void Application::EndFrame() {
		m_Renderer2D.value().EndFrame();
		m_GizmoRenderer.value().EndFrame();
		m_Window.value().SwapBuffers();
		Input::Update();
	}

	void Application::EndFixedFrame() {

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
		m_Window.emplace(Window(GLFWWindowProperties(800,800, "Hello World", true, true, false)));
		m_Window.value().SetVsync(false);
		m_Window.value().SetWindowResizeable(true);

		OpenGL::Initialize(GLInitProperties(Color::Background(), true, GLCullingMode::GLBack));

		m_Renderer2D.emplace(Renderer2D());
		m_Renderer2D.value().Initialize();

		m_GizmoRenderer.emplace();
		m_GizmoRenderer.value().Initialize();

		m_PhysicsSystem.emplace(PhysicsSystem());

		TextureManager::Initialize();
		SceneManager::Initialize();
	}
}