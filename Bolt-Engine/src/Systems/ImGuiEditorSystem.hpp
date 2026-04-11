#pragma once
#include "Core/Layer.hpp"
#include "Core/Export.hpp"
#include "Collections/Color.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"
#include "Collections/Ids.hpp"
#include "Collections/Viewport.hpp"
#include "Core/Log.hpp"
#include "Gui/AssetBrowser.hpp"
#include "Gui/PackageManagerPanel.hpp"
#include "Packages/PackageManager.hpp"
#include "Editor/EditorCamera.hpp"


#include <string>
#include <vector>
#include <chrono>

namespace Bolt {
	class BOLT_API ImGuiEditorSystem : public Layer {
	public:
		using Layer::Layer;

		void OnAttach(Application& app) override;
		void OnDetach(Application& app) override;
		void OnImGuiRender(Application& app) override;
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
		void RenderEntitiesPanel();
		void RenderInspectorPanel(Scene& scene);
		void RenderEditorView(Scene& scene);
		void RenderGameView(Scene& scene);
		void RenderLogPanel();
		void RenderProjectPanel();
		void RenderBuildPanel();
		void RenderPlayerSettingsPanel();
		void ExecuteBuild();
		void RenderPackageManagerPanel();
		void RenderAssetInspector();
		void RestoreEditorSceneAfterPlaymode();

		void RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
			const glm::mat4& vp, const AABB& viewportAABB,
			bool withGizmos, const Color& clearColor = Color::Background());

		EntityHandle m_SelectedEntity = entt::null;
		EventId m_LogSubscriptionId{};
		std::vector<LogEntry> m_LogEntries;
		bool m_ShowLogInfo = true;
		bool m_ShowLogWarn = true;
		bool m_ShowLogError = true;

		// Entity ordering for hierarchy drag-reorder
		std::vector<entt::entity> m_EntityOrder;

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

		std::string m_PendingSceneFileDrop;
		std::string m_PendingSceneSwitch;
		std::string m_ConfirmDialogPendingPath;
		bool m_ShowSaveConfirmDialog = false;
		bool m_InspectorItemWasActive = false;
		char m_ComponentSearchBuffer[128]{};
		std::string m_SelectedAssetPath;

		std::string m_PlayModeScenePath;
		int m_StepFrames = 0;

		bool m_ShowQuitSaveDialog = false;
		bool m_ShowBuildPanel = false;
		bool m_ShowPlayerSettings = false;
		bool m_ShowPackageManager = false;

		// Scene list for build
		std::vector<std::string> m_BuildSceneList;
		bool m_BuildSceneListInitialized = false;
		int m_DraggedSceneIndex = -1;
		bool m_PackageManagerInitialized = false;
		PackageManager m_PackageManager;
		PackageManagerPanel m_PackageManagerPanel;
		std::string m_BuildOutputDir;
		char m_BuildOutputDirBuffer[512]{};
		int m_BuildState = 0; // 0=idle, 1=pending (render overlay), 2=execute
		bool m_BuildAndPlay = false;
		std::vector<entt::entity> m_EditorPausedAudioEntities; // AudioSources paused by editor, not by gameplay
		std::chrono::steady_clock::time_point m_BuildStartTime;
	};
}
