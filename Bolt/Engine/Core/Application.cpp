#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Graphics/TextureManager.hpp"
#include "../Graphics/OpenGL.hpp"
#include "../Core/SingleInstance.hpp"
#include "../Audio/AudioManager.hpp"
#include "Time.hpp"
#include "Input.hpp"


namespace Bolt {
	float Application::s_TargetFramerate = 144;
	float Application::s_MaxPossibleFPS = 0;
	const double Application::k_PausedTargetFrameRate = 10;

	Event<> Application::OnApplicationQuit;


	bool Application::s_ShouldQuit = false;
	bool Application::s_IsPaused = false;

	Application* Application::s_Instance = nullptr;
	bool Application::s_ForceSingleInstance = false;
	std::string Application::s_Name = "App";

	void Application::Run()
	{
		if (s_ForceSingleInstance) {
			static SingleInstance instance(s_Name);

			if (instance.IsAlreadyRunning())
			{
				Logger::Error("Application", "An Instance of this app is already running!");
				return;
			}
		}

		Logger::Message("Initializing Application");
		Timer timer = Timer::Start();
		Initialize();
		Logger::Message("Application", "Initialization took " + timer.ToString());

		while (!m_Window.value().ShouldClose() && !s_ShouldQuit) {
			using Clock = std::chrono::high_resolution_clock;
			using Duration = std::chrono::high_resolution_clock::duration;

			static float fixedUpdateAccumulator = 0.0f;

			double targetFrameRate = s_IsPaused ? k_PausedTargetFrameRate : s_TargetFramerate;
			Duration TARGET_FRAME_TIME = std::chrono::duration_cast<Duration>(std::chrono::duration<double>(1.0 / targetFrameRate));
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
					if (!s_IsPaused) {
						BeginFixedFrame();
						EndFixedFrame();
					}
				}
				catch (std::runtime_error e) {
					Logger::Error(e.what());
				}

				fixedUpdateAccumulator -= Time::s_FixedDeltaTime;
			}

			BeginFrame();
			EndFrame();

			if (Input::GetKey(KeyCode::D) && Input::GetKey(KeyCode::LeftSuper))
				Logger::Message("Minimize");
			else
			{
				glfwPollEvents();
			}

			lastTime = frameStart;
		}

		Shutdown();
	}


	void Application::BeginFrame() {
		CoreInput();

		if (!s_IsPaused) {
			AudioManager::Update();
			SceneManager::UpdateScenes();
			m_Renderer2D.value().BeginFrame();
			m_GizmoRenderer.value().BeginFrame();
		}
		else {
			SceneManager::OnApplicationPaused();
		}

	}

	void Application::BeginFixedFrame() {
		SceneManager::FixedUpdateScenes();
		m_PhysicsSystem.value().FixedUpdate(Time::GetFixedDeltaTime());
	}

	void Application::EndFrame() {
		if (!s_IsPaused) {
			m_Renderer2D.value().EndFrame();
			m_GizmoRenderer.value().EndFrame();
			m_Window.value().SwapBuffers();
		}

		Input::Update();
		Time::s_FrameCount++;
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
		Timer timer = Timer::Start();
		m_Window.emplace(Window(GLFWWindowProperties(800, 800, "Hello World", true, true, false)));
		m_Window.value().SetVsync(true);
		m_Window.value().SetWindowResizeable(true);
		Logger::Message("GLFWWindow", "Initialization took " + timer.ToString());

		timer.Reset();
		OpenGL::Initialize(GLInitProperties2D(Color::Background(), GLCullingMode::GLBack));
		Logger::Message("OpenGL", "Initialization took " + timer.ToString());

		timer.Reset();
		m_Renderer2D.emplace(Renderer2D());
		m_Renderer2D.value().Initialize();
		Logger::Message("Renderer2D", "Initialization took " + timer.ToString());

		timer.Reset();
		m_GizmoRenderer.emplace();
		m_GizmoRenderer.value().Initialize();
		Logger::Message("GizmoRenderer", "Initialization took " + timer.ToString());

		timer.Reset();
		m_PhysicsSystem.emplace(PhysicsSystem());
		Logger::Message("PhysicsSystem", "Initialization took " + timer.ToString());

		timer.Reset();
		TextureManager::Initialize();
		Logger::Message("TextureManager", "Initialization took " + timer.ToString());

		timer.Reset();
		//AudioManager::Initialize();
		Logger::Message("AudioManager", "Initialization took " + timer.ToString());

		timer.Reset();
		// Initialize as last since it calls Awake() + Start() on all systems
		SceneManager::Initialize();
		Logger::Message("SceneManager", "Initialization took " + timer.ToString());
	}

	void Application::Shutdown() {
		OnApplicationQuit.Invoke();

		m_PhysicsSystem.value().Shutdown();
		m_GizmoRenderer.value().Shutdown();
		m_Renderer2D.value().Shutdown();

		m_Window.value().Destroy();
	}
}