#pragma once
#include "../Graphics/Renderer2D.hpp"
#include "../Graphics/GizmoRenderer.hpp"
#include "Window.hpp"
#include <optional>
#include "../Physics/PhysicsSystem.hpp"
#include  "../Utils/Event.hpp"

namespace Bolt {

	class Application {
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

		static float GetTargetFramerate() { return s_TargetFramerate; }
		static float GetMaxPossibleFPS() { return s_MaxPossibleFPS; }

		static void SetTargetFramerate(float framerate) { s_TargetFramerate = framerate; }

		static void SetForceSingleInstance(bool value) {
			s_ForceSingleInstance = value;
		}
		static bool GetForceSingleInstance() {
			return s_ForceSingleInstance;
		}

		static Application* GetInstance() { return s_Instance; }

		static void Quit() { s_ShouldQuit = true; }
		static void Pause(bool paused) {s_IsPaused = paused;}
		static const bool IsPaused() { return s_IsPaused; }
		static Window& GetWindow() { return s_Instance->m_Window.value(); }

		static Event<> OnApplicationQuit;
		static Event<> OnApplicationInitialize;

		static Event<bool> OnApplicationFocus;
		static Event<bool> OnApplicationPause;

		static Event<> OnApplicationBeginFrame;
		static Event<> OnApplicationEndFrame;

	private:
		static std::string s_Name;

		static bool s_ForceSingleInstance;
		static bool s_ShouldQuit;
		static bool s_IsPaused;

		static const double k_PausedTargetFrameRate;

		void Initialize();
		void Shutdown();
		void CoreInput();

		static Application* s_Instance;

		std::optional<Window> m_Window;
		std::optional<Renderer2D> m_Renderer2D;
		std::optional<GizmoRenderer> m_GizmoRenderer;
		std::optional<PhysicsSystem> m_PhysicsSystem;
		static std::shared_ptr<Viewport> s_Viewport;

		static float s_TargetFramerate;
		static float s_MaxPossibleFPS;

		friend class Window;
	};
}