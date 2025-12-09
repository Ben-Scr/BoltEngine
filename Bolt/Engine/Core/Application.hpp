#pragma once
#include "../Graphics/Renderer2D.hpp"
#include "../Graphics/GizmoRenderer.hpp"
#include "Window.hpp"
#include <optional>
#include "../Physics/PhysicsSystem.hpp"

#include <mutex>

namespace Bolt {

	class Application {
	public:
		Application() { Application::s_Instance = this; };
		void Run();
		void BeginFrame();
		void BeginFixedFrame();
		void EndFixedFrame();
		void EndFrame();
		
		static float GetTargetFramerate() { return s_TargetFramerate; }
		static float GetMaxPossibleFPS() { return s_MaxPossibleFPS; }

		static void SetTargetFramerate(float framerate) { s_TargetFramerate = framerate; }

		static void SetForceSingleInstance(bool value) {
			s_ForceSingleInstance = value;
		}
		static bool GetForceSingleInstance() {
			return s_ForceSingleInstance;
		}

		static Application* GetInstance() { return s_Instance; };

	private:
		static bool s_ForceSingleInstance;

		void Initialize();
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