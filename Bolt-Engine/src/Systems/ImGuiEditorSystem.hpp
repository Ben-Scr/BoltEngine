#pragma once
#include "Core/Application.hpp"
#include  "Scene/ISystem.hpp"
#include "Core/Export.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"
#include "Collections/Ids.hpp"
#include "Collections/Viewport.hpp"
#include "Debugging/Logger.hpp"
#include "Core/Log.hpp"

#include <string>
#include <vector>

namespace Bolt {
	class BOLT_API ImGuiEditorSystem : public ISystem {
	public:
		void Awake(Scene& scene) override;
		void OnDestroy(Scene& scene) override;
		void OnGui(Scene& scene) override;
	private:
		struct LogEntry {
			std::string Message;
			Log::Level Level;
		};

		void EnsureViewportFramebuffer(int width, int height);
		void DestroyViewportFramebuffer();
		void RenderDockspaceRoot();
		void RenderMainMenu(Scene& scene);
		void RenderToolbar();
		void RenderEntitiesPanel(Scene& scene);
		void RenderInspectorPanel(Scene& scene);
		void RenderViewportPanel(Scene& scene);
		void RenderLogPanel();

		EntityHandle m_SelectedEntity = entt::null;
		EventId m_LogSubscriptionId{};
		std::vector<LogEntry> m_LogEntries;

		unsigned int m_ViewportFramebufferId = 0;
		unsigned int m_ViewportColorTextureId = 0;
		unsigned int m_ViewportDepthRenderBufferId = 0;
		Viewport m_EditorViewport{ 1, 1 };
		bool m_IsViewportHovered = false;
		bool m_IsViewportFocused = false;
		bool m_IsPlaying = false;
	};
}