#include "pch.hpp"
#include "Application.hpp"
#include "Scene/SceneManager.hpp"
#include "Graphics/TextureManager.hpp"
#include "Graphics/OpenGL.hpp"
#include "Core/SingleInstance.hpp"
#include "Audio/AudioManager.hpp"
#include <Utils/Timer.hpp>
#include <Utils/StringHelper.hpp>

#include "Input.hpp"
#include <GLFW/glfw3.h>

namespace Bolt {
	const float Application::k_PausedTargetFrameRate = 10;
	std::string Application::s_Name = "App";

	Application* Application::s_Instance = nullptr;

	void Application::Run()
	{
		if (m_ForceSingleInstance) {
			static SingleInstance instance(s_Name);
			BT_ASSERT(!instance.IsAlreadyRunning(), BoltErrorCode::Undefined, "An Instance of this app is already running!");
		}

		BT_INFO("Initializing Application");
		Timer timer = Timer();
		Initialize();
		BT_INFO("Application Initialization took " + StringHelper::ToString(timer));
		Start();
		m_LastFrameTime = Clock::now();

		while (m_Window && !m_Window->ShouldClose() && !m_ShouldQuit) {
			const float targetFps = Max(GetTargetFramerate(), 1.0f);
			DurationChrono targetFrameTime = std::chrono::duration_cast<DurationChrono>(std::chrono::duration<double>(1.0 / targetFps));
			auto now = Clock::now();

			// Info: CPU idling for fps cap if vsync is disabled, or for paused frame cap.
			if (!m_Window->IsVsync() || m_IsPaused)
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
			m_MaxPossibleFPS = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;

			if (deltaTime >= 0.25f) {
				ResetTimePoints();
				deltaTime = 0.0f;
			}

			m_Time.Update(deltaTime);

			m_FixedUpdateAccumulator += m_Time.GetDeltaTime();
			while (m_FixedUpdateAccumulator >= m_Time.GetUnscaledFixedDeltaTime()) {
				try {
					if (!m_IsPaused) {

						BeginFixedFrame();
						EndFixedFrame();
					}
				}
				catch (const std::exception& e) {
					BT_ERROR_TAG("FixedUpdate", e.what());
					m_ShouldQuit = true;
					break;
				}

				m_FixedUpdateAccumulator -= m_Time.GetUnscaledFixedDeltaTime();
			}

			BeginFrame();
			EndFrame();

			glfwPollEvents();

			m_LastFrameTime = frameStart;
			m_Time.AdvanceFrameCount();
		}

		Shutdown();

		if (m_CanReload) {
			m_CanReload = false;
			Run();
		}
	}

	void Application::Initialize() {
		m_Configuration = GetConfiguration();

		Timer timer = Timer();
		Window::Initialize();
		m_Window = std::make_unique<Window>(m_Configuration.WindowSpecification);
		m_Window->SetVsync(m_Configuration.Vsync);
		m_Window->SetWindowResizeable(m_Configuration.WindowSpecification.Resizeable);
		BT_INFO("Window", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		OpenGL::Initialize(GLInitProperties2D(Color::Background(), GLCullingMode::GLBack));
		BT_INFO("OpenGL", "Initialization took " + StringHelper::ToString(timer));

		timer.Reset();
		m_Renderer2D = std::make_unique<Renderer2D>();
		m_Renderer2D->Initialize();
		BT_INFO("Renderer2D", "Initialization took " + StringHelper::ToString(timer));

		if (m_Configuration.EnableGizmoRenderer) {
			timer.Reset();
			m_GizmoRenderer2D = std::make_unique<GizmoRenderer2D>();
			m_GizmoRenderer2D->Initialize();
			BT_INFO("GizmoRenderer", "Initialization took " + StringHelper::ToString(timer));
		}

		if (m_Configuration.EnableImGui) {
			timer.Reset();
			m_ImGuiRenderer = std::make_unique<ImGuiRenderer>();
			m_ImGuiRenderer->Initialize(m_Window->GetGLFWWindow());
			BT_INFO("ImGuiRenderer", "Initialization took " + StringHelper::ToString(timer));
		}

		if (m_Configuration.EnableGuiRenderer) {
			timer.Reset();
			m_GuiRenderer = std::make_unique<GuiRenderer>();
			m_GuiRenderer->Initialize();
			BT_INFO("GuiRenderer", "Initialization took " + StringHelper::ToString(timer));
		}

		if (m_Configuration.EnablePhysics2D) {
			timer.Reset();
			m_PhysicsSystem2D = std::make_unique<PhysicsSystem2D>();
			m_PhysicsSystem2D->Initialize();
			BT_INFO("PhysicsSystem", "Initialization took " + StringHelper::ToString(timer));
		}

		timer.Reset();
		TextureManager::Initialize();
		BT_INFO("TextureManager", "Initialization took " + StringHelper::ToString(timer));

		if (m_Configuration.EnableAudio) {
			timer.Reset();
			AudioManager::Initialize();
			BT_INFO("AudioManager", "Initialization took " + StringHelper::ToString(timer));
		}

		timer.Reset();

		// Info: Initialize as last since it calls Awake() + Start() on all systems which can use classes such as TextureManager
		ConfigureScenes();
		m_SceneManager->Initialize();
		BT_INFO("SceneManager", "Initialization took " + StringHelper::ToString(timer));

		if (m_Configuration.SetWindowIcon) {
			try {
				auto handle = TextureManager::LoadTexture("icon.png");
				auto texture = TextureManager::GetTexture(handle);
				m_Window->SetWindowIcon(texture);
			}
			catch (const std::runtime_error& e) {
				BT_ERROR_TAG("Window", e.what());
			}
		}
	}


	void Application::BeginFrame() {
		CoreInput();

		if (!m_IsPaused) {
			if (m_IsPlaying && m_Configuration.EnableAudio) AudioManager::Update();
			Update();

			if (m_IsPlaying && m_SceneManager) m_SceneManager->UpdateScenes();

			if (m_Renderer2D)
				BOLT_TRY_CATCH_LOG(m_Renderer2D->BeginFrame());

			if (m_ImGuiRenderer) {
				BOLT_TRY_CATCH_LOG(m_ImGuiRenderer->BeginFrame());
				if (m_SceneManager) m_SceneManager->OnGuiScenes();
			}

			if (m_GuiRenderer)
				BOLT_TRY_CATCH_LOG(m_GuiRenderer->BeginFrame(*m_SceneManager));

			if (m_GizmoRenderer2D)
				BOLT_TRY_CATCH_LOG(m_GizmoRenderer2D->BeginFrame());
		}
		else {
			OnPaused();
		}
	}

	void Application::Quit() {
		if (s_Instance) s_Instance->m_ShouldQuit = true;
	}

	void Application::BeginFixedFrame() {
		FixedUpdate();

		if (!m_IsPlaying) return;

		if (m_SceneManager) m_SceneManager->FixedUpdateScenes();
		if (m_PhysicsSystem2D) m_PhysicsSystem2D->FixedUpdate(m_Time.GetFixedDeltaTime());
	}

	void Application::EndFrame() {
		if (!m_IsPaused) {
			RenderPipelineOnly();
		}

		m_Input.Update();
	}

	void Application::RenderPipelineOnly() {
		if (m_Renderer2D)
			BOLT_TRY_CATCH_LOG(m_Renderer2D->EndFrame());

		if (m_ImGuiRenderer)
			BOLT_TRY_CATCH_LOG(m_ImGuiRenderer->EndFrame());

		if (m_GuiRenderer)
			BOLT_TRY_CATCH_LOG(m_GuiRenderer->EndFrame());
		if (m_GizmoRenderer2D)
			BOLT_TRY_CATCH_LOG(m_GizmoRenderer2D->EndFrame());

		if (m_Window) m_Window->SwapBuffers();
	}

	void Application::RenderOnceForRefresh() {
		if (m_Renderer2D) {
			BOLT_TRY_CATCH_LOG(m_Renderer2D->BeginFrame());
			BOLT_TRY_CATCH_LOG(m_Renderer2D->EndFrame());
		}

		if (m_ImGuiRenderer) {
			BOLT_TRY_CATCH_LOG(m_ImGuiRenderer->BeginFrame());
			if (m_SceneManager) BOLT_TRY_CATCH_LOG(m_SceneManager->OnGuiScenes());
			BOLT_TRY_CATCH_LOG(m_ImGuiRenderer->EndFrame());
		}

		if (m_GuiRenderer && m_SceneManager) {
			BOLT_TRY_CATCH_LOG(m_GuiRenderer->BeginFrame(*m_SceneManager));
			BOLT_TRY_CATCH_LOG(m_GuiRenderer->EndFrame());
		}

		if (m_GizmoRenderer2D) {
			BOLT_TRY_CATCH_LOG(m_GizmoRenderer2D->BeginFrame());
			BOLT_TRY_CATCH_LOG(m_GizmoRenderer2D->EndFrame());
		}

		if (m_Window) m_Window->SwapBuffers();
	}

	void Application::EndFixedFrame() {

	}

	void Application::CoreInput() {
		if (m_Input.GetKeyDown(KeyCode::Esc)) {
			if (m_Window) m_Window->MinimizeWindow();
		}
		if (m_Input.GetKeyDown(KeyCode::F11)) {
			if (m_Window) m_Window->SetFullScreen(!m_Window->IsFullScreen());
		}
	}

	void Application::ResetTimePoints() {
		m_LastFrameTime = Clock::now();
		m_FixedUpdateAccumulator = 0;
	}


	void Application::Shutdown() {
		OnQuit();

		if (m_PhysicsSystem2D) m_PhysicsSystem2D->Shutdown();
		if (m_GizmoRenderer2D) m_GizmoRenderer2D->Shutdown();
		if (m_Renderer2D) m_Renderer2D->Shutdown();
		if (m_ImGuiRenderer) m_ImGuiRenderer->Shutdown();

		if (m_SceneManager) m_SceneManager->Shutdown();
		TextureManager::Shutdown();

		if (AudioManager::IsInitialized())
			AudioManager::Shutdown();

		if (m_Window) m_Window->Destroy();
		Window::Shutdown();

		m_ShouldQuit = false;
	}
}