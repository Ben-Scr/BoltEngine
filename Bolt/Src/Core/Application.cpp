#include "../pch.hpp"
#include "Application.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Graphics/TextureManager.hpp"
#include "../Graphics/OpenGL.hpp"
#include "../Core/SingleInstance.hpp"
#include "../Audio/AudioManager.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>



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

	void Application::Run()
	{
		if (s_ForceSingleInstance) {
			static SingleInstance instance(s_Name);
			BOLT_RETURN_IF(instance.IsAlreadyRunning(), BoltErrorCode::Undefined, "An Instance of this app is already running!");
		}

		Logger::Message("Initializing Application");
		Timer timer = Timer::Start();

		Initialize();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(m_Window->GetGLFWWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330 core");
		ImGui::StyleColorsDark();

		Logger::Message("Application", "Initialization took " + timer.ToString());

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
						if (++i > 10 && Time::GetDeltaTimeUnscaled() <= 10 && PhysicsSystem2D::IsEnabled()) {
							m_FixedUpdateAccumulator = 0;
							//PhysicsSystem2D::SetEnabled(false);
							BOLT_LOG_ERROR(BoltErrorCode::Overflow ,"Physics Overflow, Physics are enabled: " + std::to_string(PhysicsSystem2D::IsEnabled()));
						}

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

		Shutdown();

		if (s_CanReload) {
			s_CanReload = false;
			Run();
		}
	}


	void Application::BeginFrame() {
		CoreInput();

		ImGui::Begin("Debug Settings");

		static float targetFrameRate = 144;
		ImGui::SliderFloat("Target FPS", &targetFrameRate, 0.f, 244.f);
		SetTargetFramerate(targetFrameRate);

		static float timeScale = 1.0;
		ImGui::SliderFloat("Timescale", &timeScale, 0.f, 10.f);
		Time::SetTimeScale(timeScale);

		static float fixedFPS = 50.f;
		ImGui::SliderFloat("Fixed FPS", &fixedFPS, 10.f, 244.f);

		ImGui::End();

		if (!s_IsPaused) {
			AudioManager::Update();
			SceneManager::UpdateScenes();

			if (m_Renderer2D) m_Renderer2D->BeginFrame();
			if (m_GizmoRenderer2D) m_GizmoRenderer2D->BeginFrame();
		}
		else {
			SceneManager::OnApplicationPaused();
		}
	}

	void Application::BeginFixedFrame() {
		SceneManager::FixedUpdateScenes();
		m_PhysicsSystem2D->FixedUpdate(Time::GetFixedDeltaTime());
	}

	void Application::EndFrame() {
		if (!s_IsPaused) {
			if (m_Renderer2D) m_Renderer2D->EndFrame();
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
		if (Input::GetKeyDown(KeyCode::F12)) {
			if (m_Window) m_Window->SetVsync(!m_Window->IsVsync());
		}

		std::string fps = std::to_string(1.f / Time::GetDeltaTime()) + " FPS";

		if (m_Window) m_Window->SetTitle(fps);
	}

	void Application::ResetTimePoints() {
		m_LastFrameTime = Clock::now();
		m_FixedUpdateAccumulator = 0;
	}

	void Application::Initialize() {
		Timer timer = Timer::Start();
		Window::Initialize();
		m_Window = std::make_unique<Window>(Window(GLFWWindowProperties(800, 800, "Hello World", true, true, false)));
		m_Window->SetVsync(true);
		m_Window->SetWindowResizeable(true);
		Logger::Message("Window", "Initialization took " + timer.ToString());

		timer.Reset();
		OpenGL::Initialize(GLInitProperties2D(Color::Background(), GLCullingMode::GLBack));
		Logger::Message("OpenGL", "Initialization took " + timer.ToString());

		timer.Reset();
		m_Renderer2D = std::make_unique<Renderer2D>();
		m_Renderer2D->Initialize();
		Logger::Message("Renderer2D", "Initialization took " + timer.ToString());

		timer.Reset();
		m_GizmoRenderer2D = std::make_unique<GizmoRenderer2D>();
		m_GizmoRenderer2D->Initialize();
		Logger::Message("GizmoRenderer", "Initialization took " + timer.ToString());

		timer.Reset();
		m_PhysicsSystem2D = std::make_unique<PhysicsSystem2D>();
		m_PhysicsSystem2D->Initialize();
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
		SceneManager::Shutdown();
		TextureManager::Shutdown();

		if (m_Window) m_Window->Destroy();
		Window::Shutdown();

		s_ShouldQuit = false;
	}
}