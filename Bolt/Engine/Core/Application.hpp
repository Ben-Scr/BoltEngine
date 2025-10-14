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
		void EndFrame();
	private:
		void Initialize();
		std::optional<Window> m_Window;
		std::optional<Renderer2D> m_Renderer2D;
		std::optional<PhysicsSystem> m_PhysicsSystem;
		static std::shared_ptr<Viewport> s_Viewport;
	};
}