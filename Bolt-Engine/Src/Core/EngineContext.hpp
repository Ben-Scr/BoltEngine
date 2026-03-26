#pragma once

namespace Bolt {
	class Window;
	class SceneManager;
	class Renderer2D;
	class GuiRenderer;
	class ImGuiRenderer;
	class GizmoRenderer2D;
	class PhysicsSystem2D;
}

namespace Bolt {
	struct EngineContext {
		Window* Window = nullptr;
		SceneManager* SceneManager = nullptr;
		Renderer2D* Renderer2D = nullptr;
		GuiRenderer* GuiRenderer = nullptr;
		ImGuiRenderer* ImGuiRenderer = nullptr;
		GizmoRenderer2D* GizmoRenderer2D = nullptr;
		PhysicsSystem2D* Physics2D = nullptr;

		bool IsValid() const {
			return Window != nullptr && SceneManager != nullptr && Renderer2D != nullptr;
		}
	};
}