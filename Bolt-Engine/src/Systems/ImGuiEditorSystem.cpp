#include <pch.hpp>
#include "ImGuiEditorSystem.hpp"

#include <imgui.h>
#include <glad/glad.h>

#include <cstdio>

#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Graphics/OpenGL.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Graphics/GizmoRenderer.hpp"
#include "Graphics/Gizmos.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/ComponentRegistry.hpp"
#include <Scene/EntityHelper.hpp>

#include "Graphics/TextureManager.hpp"
#include "Gui/ImGuiUtils.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "Serialization/File.hpp"

namespace Bolt {


	void ImGuiEditorSystem::EnsureFBO(ViewportFBO& fbo, int width, int height) {
		if (width <= 0 || height <= 0) return;

		const bool sizeChanged = fbo.ViewportSize.GetWidth() != width || fbo.ViewportSize.GetHeight() != height;
		if (fbo.FramebufferId != 0 && !sizeChanged) return;

		DestroyFBO(fbo);
		fbo.ViewportSize.SetSize(width, height);

		glGenFramebuffers(1, &fbo.FramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.FramebufferId);

		glGenTextures(1, &fbo.ColorTextureId);
		glBindTexture(GL_TEXTURE_2D, fbo.ColorTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.ColorTextureId, 0);

		glGenRenderbuffers(1, &fbo.DepthRenderbufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo.DepthRenderbufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo.DepthRenderbufferId);

		BT_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, BoltErrorCode::InvalidHandle,
			"Viewport framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImGuiEditorSystem::DestroyFBO(ViewportFBO& fbo) {
		if (fbo.DepthRenderbufferId != 0) {
			glDeleteRenderbuffers(1, &fbo.DepthRenderbufferId);
			fbo.DepthRenderbufferId = 0;
		}
		if (fbo.ColorTextureId != 0) {
			glDeleteTextures(1, &fbo.ColorTextureId);
			fbo.ColorTextureId = 0;
		}
		if (fbo.FramebufferId != 0) {
			glDeleteFramebuffers(1, &fbo.FramebufferId);
			fbo.FramebufferId = 0;
		}
	}

	void ImGuiEditorSystem::EnsureViewportFramebuffer(int width, int height) {
		EnsureFBO(m_EditorViewFBO, width, height);
	}

	void ImGuiEditorSystem::DestroyViewportFramebuffer() {
		DestroyFBO(m_EditorViewFBO);
	}

	// ──────────────────────────────────────────────
	//  Lifecycle
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::Awake(Scene& scene) {
		Application::SetIsPlaying(false);

        auto* app = Application::GetInstance();
		if (app && app->GetRenderer2D()) {
			app->GetRenderer2D()->SetSkipBeginFrameRender(true);
		}

		// Load saved scene if project is active
		BoltProject* project = ProjectManager::GetCurrentProject();
		if (project) {
			std::string scenePath = project->GetSceneFilePath(project->LastOpenedScene);
			if (File::Exists(scenePath))
				SceneSerializer::LoadFromFile(scene, scenePath);
			else
				EntityHelper::CreateCamera2DEntity();
		}
		else {
			EntityHelper::CreateCamera2DEntity();
		}

		(void)scene;
		if (m_LogSubscriptionId.value != 0) {
			Log::OnLog.Remove(m_LogSubscriptionId);
		}

		m_LogEntries.clear();
		m_LogSubscriptionId = Log::OnLog.Add([this](const Log::Entry& entry) {
			// Only show user-facing logs in editor panel (Client + EditorConsole)
			// Core/engine logs still go to stdout but not the editor UI
			if (entry.Source == Log::Type::Core) return;

			m_LogEntries.push_back({ entry.Message, entry.Level });
			if (m_LogEntries.size() > 2000) {
				m_LogEntries.erase(m_LogEntries.begin(), m_LogEntries.begin() + 500);
			}
			});
	}

	void ImGuiEditorSystem::OnDestroy(Scene& scene) {
		(void)scene;
		if (m_LogSubscriptionId.value != 0) {
			Log::OnLog.Remove(m_LogSubscriptionId);
			m_LogSubscriptionId = EventId{};
		}

		DestroyFBO(m_EditorViewFBO);
		DestroyFBO(m_GameViewFBO);
		m_AssetBrowser.Shutdown();
	}

	// ──────────────────────────────────────────────
	//  Dockspace & Menu
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderDockspaceRoot() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("EditorDockspace", nullptr, flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspaceId = ImGui::GetID("BoltEditorDockspace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	void ImGuiEditorSystem::RenderMainMenu(Scene& scene) {
		if (!ImGui::BeginMenuBar()) {
			return;
		}

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
				BoltProject* project = ProjectManager::GetCurrentProject();
				if (project) {
					std::string scenePath = project->GetSceneFilePath(scene.GetName());
					SceneSerializer::SaveToFile(scene, scenePath);
					project->LastOpenedScene = scene.GetName();
					project->Save();
				}
			}
			if (ImGui::MenuItem("Reload Scene")) {
				SceneManager::Get().ReloadScene(scene.GetName());
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				Application::Quit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Application")) {
			if (ImGui::MenuItem("Reload App")) {
				Application::Reload();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	void ImGuiEditorSystem::RenderToolbar() {
		ImGui::Begin("Toolbar");
		if (!Application::GetIsPlaying()) {
			if (ImGui::Button("Play")) {
				Application::SetIsPlaying(true);
			}
		}
		else {
			if (ImGui::Button("Stop")) {
				Application::SetIsPlaying(false);
			}
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(Application::GetIsPlaying() ? "Runtime" : "Editor Paused");
		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Entities Panel (with rename support)
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderEntitiesPanel(Scene& scene) {
		ImGui::Begin("Entities");

		if (ImGui::BeginPopupContextWindow("EntityCreateContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				Entity created = scene.CreateEntity("Entity " + std::to_string(EntityHelper::EntitiesCount()));
				m_SelectedEntity = created.GetHandle();
			}

			if (ImGui::BeginMenu("2D Entity"))
			{
				if (ImGui::BeginMenu("Sprite"))
				{
					if (ImGui::MenuItem("Square"))
					{
						Entity created = scene.CreateEntity("Square " + std::to_string(EntityHelper::EntitiesCount()));
						created.AddComponent<SpriteRendererComponent>();
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("Circle"))
					{
						Entity created = scene.CreateEntity("Circle " + std::to_string(EntityHelper::EntitiesCount()));
						auto& sprite = created.AddComponent<SpriteRendererComponent>();
						sprite.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::Circle);
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("9Sliced"))
					{
						Entity created = scene.CreateEntity("9Sliced " + std::to_string(EntityHelper::EntitiesCount()));
						auto& sprite = created.AddComponent<SpriteRendererComponent>();
						sprite.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::_9Sliced);
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("Pixel"))
					{
						Entity created = scene.CreateEntity("Pixel " + std::to_string(EntityHelper::EntitiesCount()));
						auto& sprite = created.AddComponent<SpriteRendererComponent>();
						sprite.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::Pixel);
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("Logo"))
					{
						Entity created = scene.CreateEntity("Logo " + std::to_string(EntityHelper::EntitiesCount()));
						auto& sprite = created.AddComponent<SpriteRendererComponent>();
						sprite.TextureHandle = TextureManager::LoadTexture("Game/logo.png");
						m_SelectedEntity = created.GetHandle();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Physics")) {
					if(ImGui::MenuItem("Dynamic Body"))
					{
						Entity created = scene.CreateEntity("Dynamic Body " + std::to_string(EntityHelper::EntitiesCount()));
						created.AddComponent<SpriteRendererComponent>();
						created.AddComponent<Rigidbody2DComponent>();
						created.AddComponent<BoxCollider2DComponent>();
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("Kinematic Body"))
					{
						Entity created = scene.CreateEntity("Kinematic Body " + std::to_string(EntityHelper::EntitiesCount()));
						created.AddComponent<SpriteRendererComponent>();
						created.AddComponent<Rigidbody2DComponent>().SetBodyType(BodyType::Kinematic);
						created.AddComponent<BoxCollider2DComponent>();
						m_SelectedEntity = created.GetHandle();
					}
					if (ImGui::MenuItem("Static Body"))
					{
						Entity created = scene.CreateEntity("Static Body " + std::to_string(EntityHelper::EntitiesCount()));
						created.AddComponent<SpriteRendererComponent>();
						created.AddComponent<Rigidbody2DComponent>().SetBodyType(BodyType::Static);
						created.AddComponent<BoxCollider2DComponent>();
						m_SelectedEntity = created.GetHandle();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Effects"))
				{
					if (ImGui::MenuItem("Particle System"))
					{
						Entity created = scene.CreateEntity("Particle System " + std::to_string(EntityHelper::EntitiesCount()));
						created.AddComponent<ParticleSystem2DComponent>();
						m_SelectedEntity = created.GetHandle();
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera"))
			{
				Entity created = scene.CreateEntity("Camera " + std::to_string(EntityHelper::EntitiesCount()));
				created.AddComponent<Camera2DComponent>();
				m_SelectedEntity = created.GetHandle();
			}

			ImGui::EndPopup();
		}

		// Scene root node
		ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
		bool sceneOpen = ImGui::TreeNodeEx(scene.GetName().c_str(), sceneFlags);

		if (!sceneOpen) {
			ImGui::End();
			return;
		}

		auto view = scene.GetRegistry().view<entt::entity>();
		for (const EntityHandle entityHandle : view) {
			Entity entity = scene.GetEntity(entityHandle);
			const bool selected = m_SelectedEntity == entityHandle;

			ImGui::PushID(static_cast<int>(static_cast<uint32_t>(entityHandle)));


			if (m_RenamingEntity == entityHandle) {
				m_EntityRenameFrameCounter++;

				ImGui::PushItemWidth(-1);
				if (m_EntityRenameFrameCounter == 1) {
					ImGui::SetKeyboardFocusHere();
				}

				bool committed = ImGui::InputText("##EntityRename", m_EntityRenameBuffer, sizeof(m_EntityRenameBuffer),
					ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

				if (committed) {
					std::string newName(m_EntityRenameBuffer);
					if (!newName.empty() && entity.HasComponent<NameComponent>()) {
						entity.GetComponent<NameComponent>().Name = newName;
					}
					m_RenamingEntity = entt::null;
					m_EntityRenameFrameCounter = 0;
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
					m_RenamingEntity = entt::null;
					m_EntityRenameFrameCounter = 0;
				}
				else if (m_EntityRenameFrameCounter > 2 && !ImGui::IsItemActive()) {
					// (Ben-Scr) Focus lost = commit
					std::string newName(m_EntityRenameBuffer);
					if (!newName.empty() && entity.HasComponent<NameComponent>()) {
						entity.GetComponent<NameComponent>().Name = newName;
					}
					m_RenamingEntity = entt::null;
					m_EntityRenameFrameCounter = 0;
				}

				ImGui::PopItemWidth();
			}
			else {
				if (ImGui::Selectable(entity.GetName().c_str(), selected)) {
					m_SelectedEntity = entityHandle;
				}

				// (Ben-Scr) Double-click to rename
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					m_RenamingEntity = entityHandle;
					m_EntityRenameFrameCounter = 0;
					std::snprintf(m_EntityRenameBuffer, sizeof(m_EntityRenameBuffer), "%s", entity.GetName().c_str());
				}
			}

			if (ImGui::BeginPopupContextItem())
			{
				m_SelectedEntity = entityHandle;

				if (ImGui::MenuItem("Delete Entity"))
				{
					scene.DestroyEntity(entity);
					if (m_SelectedEntity == entityHandle)
						m_SelectedEntity = entt::null;
					if (m_RenamingEntity == entityHandle)
						m_RenamingEntity = entt::null;
				}

				if (ImGui::MenuItem("Copy Entity"))
				{
					Entity copy = scene.CreateEntity(entity.GetName() + " (Copy)");

					const auto& registry = SceneManager::Get().GetComponentRegistry();
					registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
						if (info.category != ComponentCategory::Component) return;
						if (!info.has(entity)) return;
						if (info.has(copy)) return;
						info.add(copy);
					});

					m_SelectedEntity = copy.GetHandle();
				}

				if (ImGui::MenuItem("Rename"))
				{
					m_RenamingEntity = entityHandle;
					m_EntityRenameFrameCounter = 0;
					std::snprintf(m_EntityRenameBuffer, sizeof(m_EntityRenameBuffer), "%s", entity.GetName().c_str());
				}

				ImGui::EndPopup();
			}

			ImGui::PopID();
		}

		ImGui::TreePop(); // Scene root node

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Inspector Panel
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderInspectorPanel(Scene& scene) {
		ImGui::Begin("Inspector");

		if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
			m_SelectedEntity = entt::null;
			ImGui::TextDisabled("No entity selected");
			ImGui::End();
			return;
		}

		Entity entity = scene.GetEntity(m_SelectedEntity);
		std::string entityName = entity.GetName();
		ImGui::Text("Entity: %s", entityName.c_str());
		ImGui::Separator();

		const auto& registry = SceneManager::Get().GetComponentRegistry();

		std::type_index pendingRemoval = typeid(void);

		registry.ForEachComponentInfo([&](const std::type_index& typeId, const ComponentInfo& info) {
			if (info.category != ComponentCategory::Component) return;
			if (!info.has(entity)) return;

			bool removeRequested = false;
			bool open = ImGuiUtils::BeginComponentSection(info.displayName.c_str(), removeRequested);

			if (removeRequested) {
				pendingRemoval = typeId;
			}

			if (open) {
				if (info.drawInspector) {
					info.drawInspector(entity);
				}
				ImGuiUtils::EndComponentSection();
			}
		});

		if (pendingRemoval != typeid(void)) {
			registry.ForEachComponentInfo([&](const std::type_index& typeId, const ComponentInfo& info) {
				if (typeId == pendingRemoval && info.remove) {
					info.remove(entity);
				}
			});
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0))) {
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup")) {
			registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
				if (info.category != ComponentCategory::Component) return;
				if (info.has(entity)) return;

				if (ImGui::MenuItem(info.displayName.c_str())) {
					info.add(entity);
				}
			});
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Render scene into an FBO with a given camera
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
		const glm::mat4& vp, const AABB& viewportAABB,
		bool withGizmos)
	{
		auto* app = Application::GetInstance();
		if (!app) return;
		auto* renderer = app->GetRenderer2D();
		if (!renderer) return;

		int w = fbo.ViewportSize.GetWidth();
		int h = fbo.ViewportSize.GetHeight();

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.FramebufferId);
		glViewport(0, 0, w, h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SceneManager::Get().ForeachLoadedScene([&](const Scene& s) {
			renderer->RenderSceneWithVP(s, vp, viewportAABB);
		});

		if (withGizmos && Gizmo::IsEnabled()) {
			GizmoRenderer2D::RenderWithVP(vp);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto* window = Application::GetWindow();
		if (window) {
			glViewport(0, 0, window->GetWidth(), window->GetHeight());
		}
	}

	// ──────────────────────────────────────────────
	//  Editor View — free-move editor camera + gizmos
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderEditorView(Scene& scene) {
		ImGui::Begin("Editor View");

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		const int fbW = static_cast<int>(viewportSize.x);
		const int fbH = static_cast<int>(viewportSize.y);

		if (fbW > 0 && fbH > 0) {
			EnsureFBO(m_EditorViewFBO, fbW, fbH);
			m_EditorCamera.SetViewportSize(fbW, fbH);

			if (m_EditorViewFBO.FramebufferId != 0) {
				auto* app = Application::GetInstance();
				if (app) {
					auto& input = app->GetInput();
					float dt = app->GetTime().GetDeltaTimeUnscaled();

					Vec2 mouseDelta = { 0.0f, 0.0f };
					if (m_IsEditorViewHovered && input.GetMouse(MouseButton::Middle)) {
						mouseDelta = input.GetMouseDelta();
					}
					float scroll = m_IsEditorViewHovered ? input.ScrollValue() : 0.0f;

					m_EditorCamera.Update(dt, m_IsEditorViewHovered, mouseDelta, scroll);
				}

				glm::mat4 vp = m_EditorCamera.GetViewProjectionMatrix();
				AABB viewAABB = m_EditorCamera.GetViewportAABB();

				RenderSceneIntoFBO(m_EditorViewFBO, scene, vp, viewAABB, true);

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_EditorViewFBO.ColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));
			}
		}
		else {
			ImGui::TextDisabled("Editor View has no drawable area");
		}

		m_IsEditorViewHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
		m_IsEditorViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Game View — renders with the in-game camera
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderGameView(Scene& scene) {
		(void)scene;
		ImGui::Begin("Game View");

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		const int fbW = static_cast<int>(viewportSize.x);
		const int fbH = static_cast<int>(viewportSize.y);

		if (fbW > 0 && fbH > 0) {
			EnsureFBO(m_GameViewFBO, fbW, fbH);

			Camera2DComponent* gameCam = Camera2DComponent::Main();
			if (m_GameViewFBO.FramebufferId != 0 && gameCam && gameCam->IsValid()) {
				Viewport* savedViewport = gameCam->GetViewport();
				int savedW = savedViewport->GetWidth();
				int savedH = savedViewport->GetHeight();
				savedViewport->SetSize(fbW, fbH);

				gameCam->UpdateViewport();
				glm::mat4 vp = gameCam->GetViewProjectionMatrix();
				AABB viewAABB = gameCam->GetViewportAABB();

				RenderSceneIntoFBO(m_GameViewFBO, scene, vp, viewAABB, false);

				savedViewport->SetSize(savedW, savedH);

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_GameViewFBO.ColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));
			}
			else {
				ImGui::TextDisabled("No main camera in scene");
			}
		}
		else {
			ImGui::TextDisabled("Game View has no drawable area");
		}

		m_IsGameViewHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
		m_IsGameViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Log Panel
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderLogPanel() {
		ImGui::Begin("Log");

		if (ImGui::Button("Clear")) {
			m_LogEntries.clear();
		}
		ImGui::Separator();

		ImGui::BeginChild("LogScroll");
		for (const LogEntry& entry : m_LogEntries) {
			ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
			if (entry.Level == Log::Level::Warn) {
				color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
			}
			else if (entry.Level == Log::Level::Error) {
				color = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(entry.Message.c_str());
			ImGui::PopStyleColor();
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Project Panel (Asset Browser)
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderProjectPanel() {
		if (!m_AssetBrowserInitialized) {
			std::string assetsRoot;
			BoltProject* project = ProjectManager::GetCurrentProject();
			if (project)
				assetsRoot = project->AssetsDirectory;
			else
				assetsRoot = Path::Combine(Path::ExecutableDir(), "Assets");

			if (!Directory::Exists(assetsRoot))
				Directory::Create(assetsRoot);

			m_AssetBrowser.Initialize(assetsRoot);
			m_AssetBrowserInitialized = true;
		}

		m_AssetBrowser.Render();
	}

	// ──────────────────────────────────────────────
	//  Main OnGui entry point
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::OnGui(Scene& scene) {
		RenderDockspaceRoot();
		RenderMainMenu(scene);
		RenderToolbar();
		RenderEntitiesPanel(scene);
		RenderInspectorPanel(scene);
		RenderEditorView(scene);
		RenderGameView(scene);
		RenderProjectPanel();
		RenderLogPanel();
		ImGui::End();
	}
}
