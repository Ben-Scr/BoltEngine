#pragma once
#include "Graphics/Renderer2D.hpp"
#include "Graphics/GizmoRenderer.hpp"

#include "Physics/PhysicsSystem2D.hpp"

#include "Scene/SceneManager.hpp"
#include "Gui/ImGuiRenderer.hpp"
#include "Gui/GuiRenderer.hpp"
#include "Utils/Event.hpp"
#include "Events/BoltEvent.hpp"

#include "Input.hpp"
#include "Time.hpp"
#include "Window.hpp"
#include "Export.hpp"

#include <chrono>

namespace Bolt {
	struct ApplicationConfig {
		WindowSpecification WindowSpecification{ 800, 800, "Bolt Runtime", true, true, false };
		bool EnableImGui = true;
		bool EnableGuiRenderer = true;
		bool EnableGizmoRenderer = true;
		bool EnablePhysics2D = true;
		bool EnableAudio = true;
		bool SetWindowIcon = true;
		bool Vsync = true;
	};

	class BOLT_API Application {
		friend class Window;
		friend class SceneManager;

		using DurationChrono = std::chrono::high_resolution_clock::duration;
		using Clock = std::chrono::high_resolution_clock;

	public:
		Application()
			: m_SceneManager(std::make_unique<SceneManager>())
			, m_FixedUpdateAccumulator{ 0 } {
			Application::s_Instance = this;
		};

		virtual ~Application() {}


		void Run();

		virtual ApplicationConfig GetConfiguration() const { return {}; }
		virtual void ConfigureScenes() {}
		virtual void Start() = 0;
		virtual void Update() = 0;
		virtual void FixedUpdate() = 0;
		virtual void OnPaused() = 0;
		virtual void OnQuit() = 0;

		static void SetIsPlaying(bool enabled) { if (s_Instance) s_Instance->m_IsPlaying = enabled; }
		static bool GetIsPlaying() { if (s_Instance) return s_Instance->m_IsPlaying; return false; }

		static void SetName(const std::string& s) { s_Name = s; }
		static void SetTargetFramerate(float framerate) { if (s_Instance) s_Instance->m_TargetFramerate = framerate; }
		static void SetForceSingleInstance(bool value) { if (s_Instance) s_Instance->m_ForceSingleInstance = value; }
		static void SetRunInBackground(bool value) { if (s_Instance) s_Instance->m_RunInBackground = value; }

		static const std::string& GetName() { return s_Name; }
		static float GetTargetFramerate() { return s_Instance ? (s_Instance->m_IsPaused ? k_PausedTargetFrameRate : s_Instance->m_TargetFramerate) : 0.0f; }
		static bool GetForceSingleInstance() { return s_Instance ? s_Instance->m_ForceSingleInstance : false; }
		static bool GetRunInBackground() { return s_Instance ? s_Instance->m_RunInBackground : false; }
		static float GetMaxPossibleFPS() { return s_Instance ? s_Instance->m_MaxPossibleFPS : 0.0f; }
		static Window* GetWindow() { return s_Instance ? s_Instance->m_Window.get() : nullptr; }


		Renderer2D* GetRenderer2D() { return m_Renderer2D.get(); }
		Input& GetInput() { return m_Input; }
		Time& GetTime() { return m_Time; }
		const Time& GetTime() const { return m_Time; }

		static Application* GetInstance() { return s_Instance; }

		SceneManager* GetSceneManager() { return m_SceneManager.get(); }
		const SceneManager* GetSceneManager() const { return m_SceneManager.get(); }

		static void Quit();
		static void Pause(bool paused) { if (s_Instance) s_Instance->m_IsPaused = paused; }
		static void Reload() { if (s_Instance) { s_Instance->m_ShouldQuit = true; s_Instance->m_CanReload = true; } };
		static bool IsPaused() { return s_Instance ? s_Instance->m_IsPaused : false; }
		void RenderOnceForRefresh();
		void OnEvent(BoltEvent& event);

		std::vector<std::string> TakePendingFileDrops() {
			std::vector<std::string> paths = std::move(m_PendingFileDrops);
			m_PendingFileDrops.clear();
			return paths;
		}

	private:
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer2D> m_Renderer2D;
		std::unique_ptr<ImGuiRenderer> m_ImGuiRenderer;
		std::unique_ptr<GuiRenderer> m_GuiRenderer;
		std::unique_ptr<GizmoRenderer2D> m_GizmoRenderer2D;
		std::unique_ptr<PhysicsSystem2D> m_PhysicsSystem2D;
		std::unique_ptr<SceneManager> m_SceneManager;
		Input m_Input;
		Time m_Time;
		ApplicationConfig m_Configuration;

		static std::string s_Name;

		bool m_ForceSingleInstance = false;
		bool m_RunInBackground = false;
		bool m_ShouldQuit = false;
		bool m_CanReload = false;
		bool m_IsPaused = false;
		bool m_IsPlaying = true;

		float m_TargetFramerate = 144.0f;
		float m_MaxPossibleFPS = 0.0f;
		static const float k_PausedTargetFrameRate;

		static Application* s_Instance;


		std::vector<std::string> m_PendingFileDrops;
		double m_FixedUpdateAccumulator;
		std::chrono::steady_clock::time_point m_LastFrameTime = Clock::now();

		void Initialize();
		void Shutdown();
		void CoreInput();
		void ResetTimePoints();
		void BeginFrame();
		void BeginFixedFrame();
		void EndFixedFrame();
		void EndFrame();
		void RenderPipelineOnly();
	};

	// To be defined in client
	Application* CreateApplication();
}