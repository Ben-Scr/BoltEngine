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
#include "Packages/NuGetSource.hpp"
#include "Packages/GitHubSource.hpp"
#include "Editor/ExternalEditor.hpp"
#include "Scripting/ScriptEngine.hpp"
#include <algorithm>
#include <filesystem>

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
		m_PackageManagerPanel.Shutdown();
		m_PackageManager.Shutdown();
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
			if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, !Application::GetIsPlaying())) {
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

		if (ImGui::BeginMenu("Project")) {
			bool hasProject = ProjectManager::HasProject();
			if (ImGui::MenuItem("Build", nullptr, false, hasProject && !Application::GetIsPlaying())) {
				m_ShowBuildPanel = true;
				if (m_BuildOutputDir.empty()) {
					BoltProject* project = ProjectManager::GetCurrentProject();
					if (project) {
						m_BuildOutputDir = Path::Combine(project->RootDirectory, "Builds", "Windows");
						std::snprintf(m_BuildOutputDirBuffer, sizeof(m_BuildOutputDirBuffer), "%s", m_BuildOutputDir.c_str());
					}
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Package Manager", nullptr, false, hasProject)) {
				m_ShowPackageManager = true;
			}

			// External editor selector
			ExternalEditor::DetectEditors();
			const auto& editors = ExternalEditor::GetAvailableEditors();
			if (!editors.empty() && ImGui::BeginMenu("Script Editor")) {
				for (int i = 0; i < static_cast<int>(editors.size()); i++) {
					bool selected = (ExternalEditor::GetSelectedIndex() == i);
					if (ImGui::MenuItem(editors[i].DisplayName.c_str(), nullptr, selected)) {
						ExternalEditor::SetSelectedIndex(i);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	void ImGuiEditorSystem::RenderToolbar() {
		ImGui::Begin("Toolbar");
		if (!Application::GetIsPlaying()) {
			if (ImGui::Button("Play")) {
				Scene* active = SceneManager::Get().GetActiveScene();
				BoltProject* project = ProjectManager::GetCurrentProject();
				if (active && project) {
					std::string scenePath = project->GetSceneFilePath(active->GetName());
					if (active->IsDirty()) {
						SceneSerializer::SaveToFile(*active, scenePath);
						project->LastOpenedScene = active->GetName();
						project->Save();
					}
					m_PlayModeScenePath = scenePath;
				}
				Application::SetIsPlaying(true);
			}
		}
		else {
			if (ImGui::Button("Stop")) {
				Application::SetIsPlaying(false);
				if (!m_PlayModeScenePath.empty()) {
					Scene* active = SceneManager::Get().GetActiveScene();
					if (active) {
						SceneSerializer::LoadFromFile(*active, m_PlayModeScenePath);
					}
					m_PlayModeScenePath.clear();
				}
				m_SelectedEntity = entt::null;
				m_RenamingEntity = entt::null;
			}
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(Application::GetIsPlaying() ? "Runtime" : "Editor Paused");
		ImGui::End();
	}



	// ──────────────────────────────────────────────
	//  Entities Panel (with rename support)
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderEntitiesPanel() {
		ImGui::Begin("Entities");

		Scene* activeScene = SceneManager::Get().GetActiveScene();

		// Right-click on empty space: create entities in the active scene
		if (activeScene && ImGui::BeginPopupContextWindow("EntityCreateContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			Scene& scene = *activeScene;
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

		// Iterate all loaded scenes
		auto loadedScenes = SceneManager::Get().GetLoadedScenes();
		std::string sceneToRemove;

		for (auto& weakScene : loadedScenes) {
			auto scenePtr = weakScene.lock();
			if (!scenePtr) continue;
			Scene& scene = *scenePtr;

			ImGui::PushID(scene.GetName().c_str());

			ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			std::string sceneLabel = scene.IsDirty() ? scene.GetName() + " *" : scene.GetName();
			bool sceneOpen = ImGui::TreeNodeEx(sceneLabel.c_str(), sceneFlags);

			// Right-click context menu on scene tree node
			if (ImGui::BeginPopupContextItem()) {
				if (loadedScenes.size() > 1) {
					if (ImGui::MenuItem("Remove")) {
						sceneToRemove = scene.GetName();
					}
				} else {
					ImGui::MenuItem("Remove", nullptr, false, false);
				}
				ImGui::EndPopup();
			}

			if (sceneOpen) {
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
								scene.MarkDirty();
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
								scene.MarkDirty();
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

						if (ImGui::MenuItem("Duplicate"))
						{
							Entity clone = scene.CreateEntity(entity.GetName() + " (Clone)");

							const auto& compReg = SceneManager::Get().GetComponentRegistry();
							compReg.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
								if (info.category != ComponentCategory::Component) return;
								if (!info.has(entity)) return;
								if (info.has(clone)) return;
								info.add(clone);
							});

							m_SelectedEntity = clone.GetHandle();
							scene.MarkDirty();
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

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		// Deferred scene removal (after iteration)
		if (!sceneToRemove.empty()) {
			m_SelectedEntity = entt::null;
			m_RenamingEntity = entt::null;
			SceneManager::Get().UnloadScene(sceneToRemove);
		}

		// Drag-drop target: only present during an active drag so it doesn't block right-click menus
		if (ImGui::GetDragDropPayload() != nullptr) {
			ImVec2 avail = ImGui::GetContentRegionAvail();
			if (avail.y > 0) {
				ImGui::InvisibleButton("##SceneDropTarget", ImVec2(-1, avail.y));
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
						std::string droppedPath(static_cast<const char*>(payload->Data));
						std::string ext = std::filesystem::path(droppedPath).extension().string();
						std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
						if (ext == ".scene") {
							m_PendingSceneFileDrop = droppedPath;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
		}

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Inspector Panel
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderInspectorPanel(Scene& scene) {
		ImGui::Begin("Inspector");

		if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
			m_SelectedEntity = entt::null;
			RenderAssetInspector();
			ImGui::End();
			m_InspectorItemWasActive = false;
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
			scene.MarkDirty();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0))) {
			ImGui::OpenPopup("AddComponentPopup");
			m_ComponentSearchBuffer[0] = '\0';
		}

		if (ImGui::BeginPopup("AddComponentPopup")) {
			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextWithHint("##CompSearch", "Search components...",
				m_ComponentSearchBuffer, sizeof(m_ComponentSearchBuffer));
			ImGui::Separator();

			std::string filter(m_ComponentSearchBuffer);
			std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

			registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
				if (info.category != ComponentCategory::Component) return;
				if (info.has(entity)) return;

				if (!filter.empty()) {
					std::string lowerName = info.displayName;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
					if (lowerName.find(filter) == std::string::npos) return;
				}

				if (ImGui::MenuItem(info.displayName.c_str())) {
					info.add(entity);
					scene.MarkDirty();
				}
			});
			ImGui::EndPopup();
		}

		// Detect inspector property changes: when a widget finishes being edited, mark dirty
		bool anyActive = ImGui::IsAnyItemActive() && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		if (m_InspectorItemWasActive && !anyActive) {
			scene.MarkDirty();
		}
		m_InspectorItemWasActive = anyActive;

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Render scene into an FBO with a given camera
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
		const glm::mat4& vp, const AABB& viewportAABB,
		bool withGizmos, const Color& clearColor)
	{
		auto* app = Application::GetInstance();
		if (!app) return;
		auto* renderer = app->GetRenderer2D();
		if (!renderer) return;

		int w = fbo.ViewportSize.GetWidth();
		int h = fbo.ViewportSize.GetHeight();

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.FramebufferId);
		glViewport(0, 0, w, h);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
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

				// Editor view always uses a stable default background
				static const Color k_EditorClearColor(0.18f, 0.18f, 0.20f, 1.0f);
				RenderSceneIntoFBO(m_EditorViewFBO, scene, vp, viewAABB, true, k_EditorClearColor);

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

				// Game view uses the main camera's clear color
				RenderSceneIntoFBO(m_GameViewFBO, scene, vp, viewAABB, false, gameCam->GetClearColor());

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
	//  Build Panel
	// ──────────────────────────────────────────────

	static bool NeedsCopy(const std::filesystem::path& src, const std::filesystem::path& dest) {
		if (!std::filesystem::exists(dest)) return true;
		try {
			return std::filesystem::last_write_time(src) > std::filesystem::last_write_time(dest);
		} catch (...) { return true; }
	}

	static int CopyDirIncremental(const std::filesystem::path& srcDir, const std::filesystem::path& destDir) {
		int copied = 0;
		std::filesystem::create_directories(destDir);
		for (auto& entry : std::filesystem::recursive_directory_iterator(srcDir)) {
			auto rel = std::filesystem::relative(entry.path(), srcDir);
			auto dest = destDir / rel;
			try {
				if (entry.is_directory()) {
					std::filesystem::create_directories(dest);
				} else if (NeedsCopy(entry.path(), dest)) {
					std::filesystem::create_directories(dest.parent_path());
					std::filesystem::copy_file(entry.path(), dest, std::filesystem::copy_options::overwrite_existing);
					copied++;
				}
			} catch (...) {}
		}
		return copied;
	}

	void ImGuiEditorSystem::ExecuteBuild() {
		m_BuildStartTime = std::chrono::steady_clock::now();

		BoltProject* project = ProjectManager::GetCurrentProject();
		if (!project) { BT_ERROR_TAG("Build", "No project loaded."); return; }

		m_BuildOutputDir = std::string(m_BuildOutputDirBuffer);
		if (m_BuildOutputDir.empty()) { BT_ERROR_TAG("Build", "No output directory specified."); return; }

		BT_INFO_TAG("Build", "Starting build for '{}'...", project->Name);

		// Save current scene if dirty
		Scene* active = SceneManager::Get().GetActiveScene();
		if (active && active->IsDirty()) {
			std::string scenePath = project->GetSceneFilePath(active->GetName());
			SceneSerializer::SaveToFile(*active, scenePath);
			project->LastOpenedScene = active->GetName();
			project->Save();
			BT_INFO_TAG("Build", "Saved current scene.");
		}

		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto outDir = std::filesystem::path(m_BuildOutputDir);

		// 1. Compile C# scripts
		if (std::filesystem::exists(project->CsprojPath)) {
			BT_INFO_TAG("Build", "Compiling C# scripts...");
			std::string cmd = "dotnet build \"" + project->CsprojPath + "\" -c Release --nologo -v q 2>&1";
			int result = std::system(cmd.c_str());
			if (result != 0) {
				BT_ERROR_TAG("Build", "C# script compilation failed (exit code {}).", result);
				return;
			}
			BT_INFO_TAG("Build", "C# scripts compiled.");
		}

		// 2. Create output directory
		try {
			std::filesystem::create_directories(outDir);
		} catch (const std::exception& e) {
			BT_ERROR_TAG("Build", "Failed to create output directory: {}", e.what());
			return;
		}

		// Incremental single-file copy helper
		auto copyFile = [&](const std::filesystem::path& src, const std::filesystem::path& dest, const std::string& name) {
			try {
				if (!std::filesystem::exists(src)) {
					BT_WARN_TAG("Build", "{} not found at {}", name, src.string());
					return;
				}
				auto canonical = std::filesystem::canonical(src);
				if (NeedsCopy(canonical, dest)) {
					std::filesystem::create_directories(dest.parent_path());
					std::filesystem::copy_file(canonical, dest, std::filesystem::copy_options::overwrite_existing);
					BT_INFO_TAG("Build", "Copied {}", name);
				}
			} catch (const std::exception& e) {
				BT_WARN_TAG("Build", "Failed to copy {}: {}", name, e.what());
			}
		};

		// 3. Copy Runtime executable (renamed to project name)
		copyFile(exeDir / ".." / "Bolt-Runtime" / "Bolt-Runtime.exe",
			outDir / (project->Name + ".exe"), "runtime executable");

		// 4. Copy engine and dependency DLLs
		copyFile(exeDir / ".." / "Bolt-Runtime" / "Bolt-Engine.dll",
			outDir / "Bolt-Engine.dll", "Bolt-Engine.dll");
		auto scriptCoreDir = exeDir / ".." / "Bolt-ScriptCore";
		copyFile(scriptCoreDir / "Bolt-ScriptCore.dll", outDir / "Bolt-ScriptCore.dll", "Bolt-ScriptCore.dll");
		copyFile(scriptCoreDir / "Bolt-ScriptCore.runtimeconfig.json", outDir / "Bolt-ScriptCore.runtimeconfig.json", "ScriptCore config");
		copyFile(scriptCoreDir / "Bolt-ScriptCore.deps.json", outDir / "Bolt-ScriptCore.deps.json", "ScriptCore deps");
		copyFile(exeDir / "nethost.dll", outDir / "nethost.dll", "nethost.dll");

		// 5. Copy bolt-project.json (always overwrite — settings may have changed)
		try {
			std::filesystem::copy_file(project->ProjectFilePath, outDir / "bolt-project.json",
				std::filesystem::copy_options::overwrite_existing);
		} catch (const std::exception& e) {
			BT_WARN_TAG("Build", "Failed to copy bolt-project.json: {}", e.what());
		}

		// 6. Copy Assets directory (incremental)
		if (std::filesystem::exists(project->AssetsDirectory)) {
			int n = CopyDirIncremental(project->AssetsDirectory, outDir / "Assets");
			BT_INFO_TAG("Build", "Assets: {} file(s) updated", n);
		}

		// 7. Copy BoltAssets directory (incremental)
		if (std::filesystem::exists(project->BoltAssetsDirectory)) {
			int n = CopyDirIncremental(project->BoltAssetsDirectory, outDir / "BoltAssets");
			BT_INFO_TAG("Build", "BoltAssets: {} file(s) updated", n);
		}

		// 8. Copy compiled C# user assembly + all dependencies (NuGet DLLs, deps.json, etc.)
		{
			auto userBinDir = std::filesystem::path(project->GetUserAssemblyOutputPath()).parent_path();
			if (std::filesystem::exists(userBinDir)) {
				auto destBinDir = outDir / "bin" / "Release";
				int copied = 0;
				for (const auto& entry : std::filesystem::directory_iterator(userBinDir)) {
					if (!entry.is_regular_file()) continue;
					auto ext = entry.path().extension().string();
					// Copy DLLs, deps.json, runtimeconfig.json — skip PDBs
					if (ext == ".dll" || ext == ".json") {
						copyFile(entry.path(), destBinDir / entry.path().filename(), entry.path().filename().string());
						copied++;
					}
				}
				BT_INFO_TAG("Build", "User assemblies: {} file(s) copied", copied);
			}
		}

		// 9. Copy native script DLL (if exists)
		{
			std::string nativeDll = project->GetNativeDllPath();
			if (std::filesystem::exists(nativeDll)) {
				copyFile(nativeDll,
					outDir / "NativeScripts" / "build" / "Release" / (project->Name + "-NativeScripts.dll"),
					"native script DLL");
			}
		}

		float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_BuildStartTime).count();
		BT_INFO_TAG("Build", "Build completed in {:.2f}s -> {}", elapsed, m_BuildOutputDir);
	}

	void ImGuiEditorSystem::RenderBuildPanel() {
		if (!m_ShowBuildPanel) return;

		ImGui::Begin("Build", &m_ShowBuildPanel);

		BoltProject* project = ProjectManager::GetCurrentProject();

		if (project) {
			// Initialize scene list from project's Scenes directory
			if (!m_BuildSceneListInitialized) {
				m_BuildSceneList.clear();
				auto scenesDir = std::filesystem::path(project->ScenesDirectory);
				if (std::filesystem::exists(scenesDir)) {
					for (const auto& entry : std::filesystem::directory_iterator(scenesDir)) {
						if (!entry.is_regular_file()) continue;
						std::string ext = entry.path().extension().string();
						if (ext == ".scene")
							m_BuildSceneList.push_back(entry.path().stem().string());
					}
				}
				// Ensure startup scene is at index 0
				if (!project->StartupScene.empty()) {
					auto it = std::find(m_BuildSceneList.begin(), m_BuildSceneList.end(), project->StartupScene);
					if (it != m_BuildSceneList.end() && it != m_BuildSceneList.begin()) {
						std::string scene = *it;
						m_BuildSceneList.erase(it);
						m_BuildSceneList.insert(m_BuildSceneList.begin(), scene);
					}
				}
				m_BuildSceneListInitialized = true;
			}

			// Scene List (collapsable)
			if (ImGui::CollapsingHeader("Scene List", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent(8);
				if (m_BuildSceneList.empty()) {
					ImGui::TextDisabled("No scenes found in Assets/Scenes/");
				}
				else {
					for (int i = 0; i < static_cast<int>(m_BuildSceneList.size()); i++) {
						ImGui::PushID(i);
						bool isStartup = (i == 0);

						if (isStartup)
							ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "[Startup]");
						else
							ImGui::TextDisabled("[%d]", i);

						ImGui::SameLine();
						ImGui::Selectable(m_BuildSceneList[i].c_str());

						// Drag-drop reordering
						if (ImGui::BeginDragDropSource()) {
							ImGui::SetDragDropPayload("SCENE_LIST_ITEM", &i, sizeof(int));
							ImGui::Text("Move: %s", m_BuildSceneList[i].c_str());
							ImGui::EndDragDropSource();
						}
						if (ImGui::BeginDragDropTarget()) {
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_LIST_ITEM")) {
								int srcIndex = *static_cast<const int*>(payload->Data);
								if (srcIndex != i) {
									std::string moved = m_BuildSceneList[srcIndex];
									m_BuildSceneList.erase(m_BuildSceneList.begin() + srcIndex);
									m_BuildSceneList.insert(m_BuildSceneList.begin() + i, moved);
									// Update startup scene
									if (!m_BuildSceneList.empty()) {
										project->StartupScene = m_BuildSceneList[0];
										project->Save();
									}
								}
							}
							ImGui::EndDragDropTarget();
						}

						ImGui::PopID();
					}
				}

				// Accept scene drops from asset browser
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
						std::string droppedPath(static_cast<const char*>(payload->Data));
						std::string ext = std::filesystem::path(droppedPath).extension().string();
						if (ext == ".scene") {
							std::string sceneName = std::filesystem::path(droppedPath).stem().string();
							auto it = std::find(m_BuildSceneList.begin(), m_BuildSceneList.end(), sceneName);
							if (it == m_BuildSceneList.end()) {
								m_BuildSceneList.push_back(sceneName);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::Unindent(8);
			}

			ImGui::Spacing();

			// Build Settings (collapsable)
			if (ImGui::CollapsingHeader("Build Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent(8);

				if (ImGui::Button("Open Player Settings")) {
					m_ShowPlayerSettings = true;
				}

				ImGui::Spacing();
				ImGui::Text("Output Directory:");
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##BuildOutputDir", m_BuildOutputDirBuffer, sizeof(m_BuildOutputDirBuffer));

				ImGui::Unindent(8);
			}
		}

		ImGui::Spacing();

		bool canBuild = project && !Application::GetIsPlaying() && m_BuildState == 0;
		if (!canBuild) ImGui::BeginDisabled();
		if (ImGui::Button("Build", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			m_BuildState = 1;
			m_BuildStartTime = std::chrono::steady_clock::now();
		}
		if (!canBuild) ImGui::EndDisabled();

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Player Settings (separate window)
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderPlayerSettingsPanel() {
		if (!m_ShowPlayerSettings) return;

		ImGui::Begin("Player Settings", &m_ShowPlayerSettings);

		BoltProject* project = ProjectManager::GetCurrentProject();
		if (!project) {
			ImGui::TextDisabled("No project loaded");
			ImGui::End();
			return;
		}

		bool changed = false;

		if (ImGui::CollapsingHeader("Resolution", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(8);
			changed |= ImGui::InputInt("Width", &project->BuildWidth);
			changed |= ImGui::InputInt("Height", &project->BuildHeight);
			if (project->BuildWidth < 320) project->BuildWidth = 320;
			if (project->BuildHeight < 240) project->BuildHeight = 240;
			ImGui::Unindent(8);
		}

		if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(8);
			changed |= ImGui::Checkbox("Fullscreen", &project->BuildFullscreen);
			changed |= ImGui::Checkbox("Resizable", &project->BuildResizable);
			ImGui::Unindent(8);
		}

		if (changed) project->Save();

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Main OnGui entry point
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::OnGui(Scene& scene) {
		// Deferred build execution: state 1 -> 2 lets the overlay render one frame first
		if (m_BuildState == 1) {
			m_BuildState = 2;
		} else if (m_BuildState == 2) {
			ExecuteBuild();
			m_BuildState = 0;
		}

		// Process OS file drops — forward to AssetBrowser
		{
			auto* app = Application::GetInstance();
			if (app) {
				auto drops = app->TakePendingFileDrops();
				if (!drops.empty() && m_AssetBrowserInitialized) {
					m_AssetBrowser.OnExternalFileDrop(drops);
				}
			}
		}

		// Process deferred scene drop from Hierarchy drag-and-drop (blocked during Play Mode)
		if (!m_PendingSceneFileDrop.empty() && !Application::GetIsPlaying()) {
			std::string dropPath = m_PendingSceneFileDrop;
			m_PendingSceneFileDrop.clear();

			std::string sceneName = std::filesystem::path(dropPath).stem().string();
			auto& sm = SceneManager::Get();
			if (!sm.HasSceneDefinition(sceneName))
				sm.RegisterScene(sceneName);
			if (!sm.IsSceneLoaded(sceneName)) {
				auto weakScene = sm.LoadSceneAdditive(sceneName);
				if (auto loaded = weakScene.lock())
					SceneSerializer::LoadFromFile(*loaded, dropPath);
			}
		}

		// Process deferred scene switch (blocked during Play Mode)
		if (!m_PendingSceneSwitch.empty() && !Application::GetIsPlaying()) {
			std::string switchPath = m_PendingSceneSwitch;
			m_PendingSceneSwitch.clear();

			Scene* active = SceneManager::Get().GetActiveScene();
			if (active) {
				SceneSerializer::LoadFromFile(*active, switchPath);
				BoltProject* project = ProjectManager::GetCurrentProject();
				if (project) {
					project->LastOpenedScene = std::filesystem::path(switchPath).stem().string();
					project->Save();
				}
			}
		}

		// Intercept Asset Browser scene load (blocked during Play Mode)
		if (!Application::GetIsPlaying()) {
			std::string pendingLoad = m_AssetBrowser.TakePendingSceneLoad();
			if (!pendingLoad.empty()) {
				Scene* active = SceneManager::Get().GetActiveScene();
				if (active && active->IsDirty()) {
					m_ConfirmDialogPendingPath = pendingLoad;
					m_ShowSaveConfirmDialog = true;
				} else {
					m_PendingSceneSwitch = pendingLoad;
				}
			}
		}

		// Ctrl+S keyboard shortcut to save active scene (blocked during Play Mode)
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S) && !Application::GetIsPlaying()) {
			Scene* active = SceneManager::Get().GetActiveScene();
			BoltProject* project = ProjectManager::GetCurrentProject();
			if (active && project) {
				std::string scenePath = project->GetSceneFilePath(active->GetName());
				SceneSerializer::SaveToFile(*active, scenePath);
				project->LastOpenedScene = active->GetName();
				project->Save();
			}
		}

		RenderDockspaceRoot();
		RenderMainMenu(scene);

		// Save confirmation modal dialog
		if (m_ShowSaveConfirmDialog) {
			ImGui::OpenPopup("Save Changes?");
			m_ShowSaveConfirmDialog = false;
		}
		if (ImGui::BeginPopupModal("Save Changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			Scene* active = SceneManager::Get().GetActiveScene();
			std::string activeName = active ? active->GetName() : "Scene";
			ImGui::Text("Save changes to %s before opening?", activeName.c_str());
			ImGui::Spacing();

			if (ImGui::Button("Save", ImVec2(100, 0))) {
				if (active) {
					BoltProject* project = ProjectManager::GetCurrentProject();
					if (project) {
						std::string savePath = project->GetSceneFilePath(active->GetName());
						SceneSerializer::SaveToFile(*active, savePath);
					}
				}
				m_PendingSceneSwitch = m_ConfirmDialogPendingPath;
				m_ConfirmDialogPendingPath.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Don't Save", ImVec2(100, 0))) {
				m_PendingSceneSwitch = m_ConfirmDialogPendingPath;
				m_ConfirmDialogPendingPath.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0))) {
				m_ConfirmDialogPendingPath.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		RenderToolbar();
		RenderEntitiesPanel();
		RenderInspectorPanel(scene);
		RenderEditorView(scene);
		RenderGameView(scene);
		RenderProjectPanel();
		RenderLogPanel();
		RenderBuildPanel();
		RenderPlayerSettingsPanel();
		RenderPackageManagerPanel();

		// Build progress overlay (same pattern as ScriptSystem compilation overlay)
		if (m_BuildState > 0) {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 center(viewport->Pos.x + viewport->Size.x * 0.5f,
				viewport->Pos.y + viewport->Size.y * 0.5f);

			ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(320, 80));
			ImGui::SetNextWindowBgAlpha(0.92f);

			ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

			ImGui::Begin("##BuildOverlay", nullptr, overlayFlags);
			ImGui::TextUnformatted("Building Project...");
			ImGui::Spacing();
			float elapsed = std::chrono::duration<float>(
				std::chrono::steady_clock::now() - m_BuildStartTime).count();
			ImGui::ProgressBar(fmodf(elapsed * 0.4f, 1.0f), ImVec2(-1, 0), "");
			ImGui::End();
		}

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Package Manager Panel
	// ──────────────────────────────────────────────

	// ──────────────────────────────────────────────
	//  Asset Inspector (shows info about selected asset when no entity selected)
	// ──────────────────────────────────────────────

	void ImGuiEditorSystem::RenderAssetInspector() {
		const std::string& selectedPath = m_AssetBrowser.GetSelectedPath();
		if (selectedPath.empty()) {
			ImGui::TextDisabled("No entity or asset selected");
			return;
		}

		std::filesystem::path p(selectedPath);
		if (!std::filesystem::exists(p)) {
			ImGui::TextDisabled("No entity or asset selected");
			return;
		}

		std::string name = p.filename().string();
		std::string ext = p.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		ImGui::Text("Asset: %s", name.c_str());
		ImGui::Separator();

		// File info
		ImGui::TextDisabled("Path: %s", selectedPath.c_str());

		try {
			auto fileSize = std::filesystem::file_size(p);
			if (fileSize >= 1024 * 1024)
				ImGui::TextDisabled("Size: %.2f MB", fileSize / (1024.0f * 1024.0f));
			else if (fileSize >= 1024)
				ImGui::TextDisabled("Size: %.1f KB", fileSize / 1024.0f);
			else
				ImGui::TextDisabled("Size: %llu bytes", fileSize);
		}
		catch (...) {}

		ImGui::TextDisabled("Type: %s", ext.c_str());

		// Image preview
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
			ImGui::Spacing();
			// Try to load and show a preview
			auto handle = TextureManager::LoadTexture(name);
			Texture2D* tex = TextureManager::GetTexture(handle);
			if (tex && tex->IsValid()) {
				ImGuiUtils::DrawTexturePreview(tex->GetHandle(), tex->GetWidth(), tex->GetHeight(), 128.0f);
				ImGui::Text("%.0f x %.0f", tex->GetWidth(), tex->GetHeight());
			}
		}
	}

	void ImGuiEditorSystem::RenderPackageManagerPanel() {
		if (!m_ShowPackageManager) return;

		if (!m_PackageManagerInitialized) {
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto toolPath = exeDir / ".." / ".." / ".." / "Bolt-PackageTool" / "bin" / "Release" / "net9.0" / "Bolt-PackageTool.exe";
			m_PackageManager.Initialize(toolPath.string());

			// Add default sources
			if (m_PackageManager.IsReady()) {
				m_PackageManager.AddSource(std::make_unique<NuGetSource>(m_PackageManager.GetToolPath()));
				m_PackageManager.AddSource(std::make_unique<GitHubSource>(
					m_PackageManager.GetToolPath(),
					"https://raw.githubusercontent.com/Ben-Scr/bolt-packages/main/index.json",
					"Engine Packages"));
			}

			m_PackageManagerPanel.Initialize(&m_PackageManager);
			m_PackageManagerInitialized = true;
		}

		ImGui::Begin("Package Manager", &m_ShowPackageManager);
		m_PackageManagerPanel.Render();
		ImGui::End();
	}
}
