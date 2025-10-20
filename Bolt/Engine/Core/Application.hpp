#pragma once
#include "../Graphics/Renderer2D.hpp"
#include "Window.hpp"
#include <optional>
#include "../Physics/PhysicsSystem.hpp"

namespace Bolt {

	class Application {
	public:
		void Run();
		void BeginFrame();
		void BeginFixedFrame();
		void EndFixedFrame();
		void EndFrame();

		static float GetTargetFramerate() { return s_TargetFramerate; }
		static void SetTargetFramerate(float framerate) { s_TargetFramerate = framerate; }
	private:
		void Initialize();
		void CoreInput();
		std::optional<Window> m_Window;
		std::optional<Renderer2D> m_Renderer2D;
		std::optional<PhysicsSystem> m_PhysicsSystem;
		static std::shared_ptr<Viewport> s_Viewport;
		static float s_TargetFramerate;
	};
}