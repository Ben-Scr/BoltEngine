#pragma once
#include "Graphics/Renderer2D.hpp"
#include "Graphics/GizmoRenderer.hpp"
#include "Core/Window.hpp"
#include "Physics/PhysicsSystem2D.hpp"
#include "Gui/ImGuiRenderer.hpp"
#include "Gui/GuiRenderer.hpp"
#include  "Utils/Event.hpp"
#include <chrono>

namespace Rml { class Context; }

namespace Bolt {
	struct ApplicationSpecs {
		std::string Name = "Bolt Application";
		bool ForceSingleInstance = true;
		bool RunInBackground = false;
		float TargetFramerate = 144.f;
	};

	class Application {
		friend class Window;
		using DurationChrono = std::chrono::high_resolution_clock::duration;
		using Clock = std::chrono::high_resolution_clock;

	public:
		Application() { Application::s_Instance = this; };
		Application(std::string name)
		{
			s_Name = name;
			Application::s_Instance = this;
		};

		int Run();

		static void SetName(const std::string& s) { s_Name = s; }
		static void SetTargetFramerate(float framerate) { s_TargetFramerate = framerate; }
		static void SetForceSingleInstance(bool value) { s_ForceSingleInstance = value; }
		static void SetRunInBackground(bool value) { s_RunInBackground = value; }

		static const std::string GetName() { return s_Name; }
		static float GetTargetFramerate() { return s_IsPaused ? k_PausedTargetFrameRate : s_TargetFramerate; }
		static bool GetForceSingleInstance() { return s_ForceSingleInstance; }
		static bool GetRunInBackground() { return s_RunInBackground; }
		static float GetMaxPossibleFPS() { return s_MaxPossibleFPS; }
		static Window* GetWindow() { return s_Instance->m_Window.get(); }
		static Rml::Context* GetRmlContext() { return s_RmlContext; }

		Renderer2D* GetRenderer2D() { return m_Renderer2D.get(); }
		static std::string GetVersion() { return "1.0"; }

		static Application* GetInstance() { return s_Instance; }

		static void Quit();
		static void Pause(bool paused) { s_IsPaused = paused; }
		static void Reload() { s_ShouldQuit = true; s_CanReload = true; };
		static const bool IsPaused() { return s_IsPaused; }

		static void OnApplicationQuit(Bolt::Event<>::Callback cb) { s_OnApplicationQuit.Add(cb); }
	private:
		static Event<> s_OnApplicationQuit;
		static Event<> OnApplicationInitialize;

		static std::string s_Name;

		static bool s_ForceSingleInstance;
		static bool s_RunInBackground;
		static bool s_ShouldQuit;
		static bool s_CanReload;
		static bool s_IsPaused;

		static double s_TargetFramerate;
		static double s_MaxPossibleFPS;
		static const double k_PausedTargetFrameRate;

		static Application* s_Instance;
		static Rml::Context* s_RmlContext;

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer2D> m_Renderer2D;
		std::unique_ptr<ImGuiRenderer> m_ImGuiRenderer;
		std::unique_ptr<GuiRenderer> m_GuiRenderer;
		std::unique_ptr<GizmoRenderer2D> m_GizmoRenderer2D;
		std::unique_ptr<PhysicsSystem2D> m_PhysicsSystem2D;

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
	};
}