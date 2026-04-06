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
#include "Gui/AssetBrowser.hpp"
#include "Editor/EditorCamera.hpp"

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

		struct ViewportFBO {
			unsigned int FramebufferId = 0;
			unsigned int ColorTextureId = 0;
			unsigned int DepthRenderbufferId = 0;
			Viewport ViewportSize{ 1, 1 };
		};

		void EnsureFBO(ViewportFBO& fbo, int width, int height);
		void DestroyFBO(ViewportFBO& fbo);

		void EnsureViewportFramebuffer(int width, int height);
		void DestroyViewportFramebuffer();

		void RenderDockspaceRoot();
		void RenderMainMenu(Scene& scene);
		void RenderToolbar();
		void RenderEntitiesPanel(Scene& scene);
		void RenderInspectorPanel(Scene& scene);
		void RenderEditorView(Scene& scene);
		void RenderGameView(Scene& scene);
		void RenderLogPanel();
		void RenderProjectPanel();

		void RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
			const glm::mat4& vp, const AABB& viewportAABB,
			bool withGizmos);

		EntityHandle m_SelectedEntity = entt::null;
		EventId m_LogSubscriptionId{};
		std::vector<LogEntry> m_LogEntries;

		EntityHandle m_RenamingEntity = entt::null;
		char m_EntityRenameBuffer[256]{};
		int m_EntityRenameFrameCounter = 0;

		ViewportFBO m_EditorViewFBO;
		EditorCamera m_EditorCamera;
		bool m_IsEditorViewHovered = false;
		bool m_IsEditorViewFocused = false;

		ViewportFBO m_GameViewFBO;
		bool m_IsGameViewHovered = false;
		bool m_IsGameViewFocused = false;

		Viewport m_EditorViewport{ 1, 1 };
		bool m_IsViewportHovered = false;
		bool m_IsViewportFocused = false;
		bool m_IsPlaying = false;

		AssetBrowser m_AssetBrowser;
		bool m_AssetBrowserInitialized = false;
	};
}