#include "pch.hpp"
#include "Application.hpp"
#include "Scene/SceneManager.hpp"
#include "Graphics/TextureManager.hpp"
#include "Graphics/OpenGL.hpp"
#include "Core/SingleInstance.hpp"
#include "Audio/AudioManager.hpp"

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include "Time.hpp"
#include "Input.hpp"
#include <GLFW/glfw3.h>

namespace Bolt {
	double Application::s_TargetFramerate = 144;
	double Application::s_MaxPossibleFPS = 0;
	const double Application::k_PausedTargetFrameRate = 10;

	Event<> Application::s_OnApplicationQuit;

	bool Application::s_ShouldQuit = false;
	bool Application::s_RunInBackground = false;
	bool Application::s_IsPaused = false;
	bool Application::s_CanReload = false;

	Application* Application::s_Instance = nullptr;
	bool Application::s_ForceSingleInstance = false;
	std::string Application::s_Name = "App";

	int Application::Run()
	{
		int err = 0;

		try {
			if (s_ForceSingleInstance) {
				static SingleInstance instance(s_Name);
				BOLT_ASSERT(!instance.IsAlreadyRunning(), BoltErrorCode::Undefined, "An Instance of this app is already running!");
			}


			Logger::Message("Initializing Application");
			Timer timer = Timer::Start();
			Initialize();
			Logger::Message("Application Initialization took " + StringHelper::ToString(timer));
			m_LastFrameTime = Clock::now();

			while ((!m_Window || !m_Window->ShouldClose()) && !s_ShouldQuit) {
				DurationChrono targetFrameTime = std::chrono::duration_cast<DurationChrono>(std::chrono::duration<double>(1.0 / GetTargetFramerate()));
				auto now = Clock::now();

				// Info: CPU idling for fps cut if window isn't vsync or app is paused
				if (!m_Window->IsVsync() || s_IsPaused)
				{
					auto const nextFrameTime = m_LastFrameTime + targetFrameTime;

					if (now + std::chrono::milliseconds(10) < nextFrameTime)
						std::this_thread::sleep_until(nextFrameTime - std::chrono::milliseconds(10));

					while (Clock::now() < nextFrameTime) {
						_mm_pause();
					}
				}

				auto frameStart = Clock::now();
				float deltaTime = std::chrono::duration<float>(frameStart - m_LastFrameTime).count();

				if (deltaTime >= 0.25f) {
					ResetTimePoints();
					deltaTime = 0.0f;
				}

				Time::Update(deltaTime);

				m_FixedUpdateAccumulator += Time::GetDeltaTime();

				size_t i = 0;
				while (m_FixedUpdateAccumulator >= Time::s_FixedDeltaTime) {
					try {
						if (!s_IsPaused) {

							BeginFixedFrame();
							EndFixedFrame();
						}
					}
					catch (std::runtime_error e) {
						Logger::Error(e.what());
						break;
					}

					m_FixedUpdateAccumulator -= Time::s_FixedDeltaTime;
				}

				BeginFrame();
				EndFrame();

				glfwPollEvents();

				m_LastFrameTime = frameStart;
			}
		}
		catch (const std::exception& ex) {
			Logger::Error(ex.what());
			Logger::Error("Application Crashed!");
			err = -1;
		}
		catch (...) {
			Logger::Error("Unknown Error");
			Logger::Error("Application Crashed!");
			err = -1;
		}

		try {
			Shutdown();
		}
		catch (const std::exception& ex) {
			Logger::Error("Error during shutdown: " + std::string(ex.what()));
			err = -1;
		}
		catch (...) {
			Logger::Error("Unknown error during shutdown");
			err = -1;
		}

		if (s_CanReload) {
			s_CanReload = false;
			Run();
		}

		return err;
	}


	void Application::BeginFrame() {
		CoreInput();

		if (!s_IsPaused) {
			AudioManager::Update();
			SceneManager::UpdateScenes();

			if (m_Renderer2D) m_Renderer2D->BeginFrame();

			if (m_ImGuiRenderer) {
				m_ImGuiRenderer->BeginFrame();
				SceneManager::OnGuiScenes();
			}

			if (m_GuiRenderer) m_GuiRenderer->BeginFrame();

			if (m_GizmoRenderer2D) m_GizmoRenderer2D->BeginFrame();
		}
		else {
			SceneManager::OnApplicationPaused();
		}
	}

	void Application::Quit() {
		s_ShouldQuit = true;
	}

	void Application::BeginFixedFrame() {
		SceneManager::FixedUpdateScenes();
		m_PhysicsSystem2D->FixedUpdate(Time::GetFixedDeltaTime());
	}

	void Application::EndFrame() {
		if (!s_IsPaused) {
			if (m_Renderer2D) m_Renderer2D->EndFrame();
			if (m_ImGuiRenderer) m_ImGuiRenderer->EndFrame();
			if (m_GuiRenderer) m_GuiRenderer->EndFrame();
			if (m_GizmoRenderer2D) m_GizmoRenderer2D->EndFrame();
			if (m_Window) m_Window->SwapBuffers();
		}

		Input::Update();
		Time::s_FrameCount++;
	}

	void Application::EndFixedFrame() {

	}

	void Application::CoreInput() {
		if (Input::GetKeyDown(KeyCode::Esc)) {
			if (m_Window) m_Window->MinimizeWindow();
		}
		if (Input::GetKeyDown(KeyCode::F11)) {
			if (m_Window) m_Window->SetFullScreen(!m_Window->IsFullScreen());
		}
	}

	void Application::ResetTimePoints() {
		m_LastFrameTime = Clock::now();
		m_FixedUpdateAccumulator = 0;
	}

	void Application::Initialize() {
		Timer timer = Timer::Start();
		Window::Initialize();
		m_Window = std::make_unique<Window>(Window(GLFWWindowProperties(800, 800, "Space Shooter", true, true, false)));
		m_Window->SetVsync(true);
		m_Window->SetWindowResizeable(true);
		Logger::Message("Window", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		OpenGL::Initialize(GLInitProperties2D(Color::Background(), GLCullingMode::GLBack));
		Logger::Message("OpenGL", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_Renderer2D = std::make_unique<Renderer2D>();
		m_Renderer2D->Initialize();
		Logger::Message("Renderer2D", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_GizmoRenderer2D = std::make_unique<GizmoRenderer2D>();
		m_GizmoRenderer2D->Initialize();
		Logger::Message("GizmoRenderer", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_ImGuiRenderer = std::make_unique<ImGuiRenderer>();
		m_ImGuiRenderer->Initialize(m_Window->GetGLFWWindow());
		Logger::Message("ImGuiRenderer", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_GuiRenderer = std::make_unique<GuiRenderer>();
		m_GuiRenderer->Initialize();
		Logger::Message("GuiRenderer", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_PhysicsSystem2D = std::make_unique<PhysicsSystem2D>();
		m_PhysicsSystem2D->Initialize();
		Logger::Message("PhysicsSystem", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		TextureManager::Initialize();
		Logger::Message("TextureManager", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		//AudioManager::Initialize();
		Logger::Message("AudioManager", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		// Info: Initialize as last since it calls Awake() + Start() on all systems which can use classes such as TextureManager
		SceneManager::Initialize();
		Logger::Message("SceneManager", "Initialization took " + StringHelper::ToString(timer));

		try {
			auto handle = TextureManager::LoadTexture("../Assets/Textures/icon.png");
			auto texture = TextureManager::GetTexture(handle);
			m_Window->SetWindowIcon(texture);
		}
		catch (const std::runtime_error& e) {
			Logger::Error(e.what());
		}
	}

	void Application::Shutdown() {
		s_OnApplicationQuit.Invoke();

		if (m_PhysicsSystem2D) m_PhysicsSystem2D->Shutdown();
		if (m_PhysicsSystem2D) m_GizmoRenderer2D->Shutdown();
		if (m_Renderer2D) m_Renderer2D->Shutdown();
		if (m_ImGuiRenderer) m_ImGuiRenderer->Shutdown();

		SceneManager::Shutdown();
		TextureManager::Shutdown();

		if (AudioManager::IsInitialized())
			AudioManager::Shutdown();

		if (m_Window) m_Window->Destroy();
		Window::Shutdown();

		s_ShouldQuit = false;
	}
}