#pragma once
#include "../Graphics/Renderer2D.hpp"
#include "../Graphics/GizmoRenderer.hpp"
#include "Window.hpp"
#include <optional>
#include "../Physics/PhysicsSystem.hpp"
#include  "../Utils/Event.hpp"

namespace Bolt {

	class Application {
		using DurationChrono = std::chrono::high_resolution_clock::duration;
		using Clock = std::chrono::high_resolution_clock;

	public:
		Application() { Application::s_Instance = this; };
		Application(std::string name)
		{
			s_Name = name;
			Application::s_Instance = this;
		};

		void Run();
		void BeginFrame();
		void BeginFixedFrame();
		void EndFixedFrame();
		void EndFrame();

		static void SetName(const std::string& s) { s_Name = s; }
		static const std::string GetName() { return s_Name; }

		static float GetTargetFramerate() { return s_IsPaused ? k_PausedTargetFrameRate : s_TargetFramerate; }
		static float GetMaxPossibleFPS() { return s_MaxPossibleFPS; }

		static void SetTargetFramerate(float framerate) { s_TargetFramerate = framerate; }

		static void SetForceSingleInstance(bool value) {
			s_ForceSingleInstance = value;
		}
		static bool GetForceSingleInstance() {
			return s_ForceSingleInstance;
		}

		static void OnApplicationQuit(Bolt::Event<>::Callback cb) {
			s_OnApplicationQuit.Add(cb);
		}
		static Application* GetInstance() { return s_Instance; }

		static void Quit() { s_ShouldQuit = true; }
		static void Pause(bool paused) { s_IsPaused = paused; }
		static const bool IsPaused() { return s_IsPaused; }
		static Window& GetWindow() { return *s_Instance->m_Window; }


	private:
		static Event<> s_OnApplicationQuit;
		static Event<> OnApplicationInitialize;

		static std::string s_Name;

		static bool s_ForceSingleInstance;
		static bool s_ShouldQuit;
		static bool s_IsPaused;

		static const double k_PausedTargetFrameRate;

		void Initialize();
		void Shutdown();
		void CoreInput();
		void ResetTimePoints();

		static Application* s_Instance;

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer2D> m_Renderer2D;
		std::unique_ptr<GizmoRenderer2D> m_GizmoRenderer2D;
		std::unique_ptr<PhysicsSystem2D> m_PhysicsSystem2D;

		double m_FixedUpdateAccumulator;
		std::chrono::steady_clock::time_point m_LastFrameTime = Clock::now();

		static std::shared_ptr<Viewport> s_Viewport;

		static double s_TargetFramerate;
		static double s_MaxPossibleFPS;

		friend class Window;
	};
}