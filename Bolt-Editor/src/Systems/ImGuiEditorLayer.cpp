#include <pch.hpp>
#include "ImGuiEditorLayer.hpp"

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
#include "Utils/Process.hpp"
#include "Packages/NuGetSource.hpp"
#include "Packages/GitHubSource.hpp"
#include "Editor/ExternalEditor.hpp"
#include "Gui/EditorIcons.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Scripting/ScriptSystem.hpp"
#include <algorithm>
#include <filesystem>
#include <unordered_set>

namespace Bolt {


	void ImGuiEditorLayer::EnsureFBO(ViewportFBO& fbo, int width, int height) {
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

	void ImGuiEditorLayer::DestroyFBO(ViewportFBO& fbo) {
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

	void ImGuiEditorLayer::EnsureViewportFramebuffer(int width, int height) {
		EnsureFBO(m_EditorViewFBO, width, height);
	}

	void ImGuiEditorLayer::DestroyViewportFramebuffer() {
		DestroyFBO(m_EditorViewFBO);
	}

	// ──────────────────────────────────────────────
	//  Lifecycle
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::OnAttach(Application& app) {
		Application::SetIsPlaying(false);
		if (app.GetRenderer2D()) {
			app.GetRenderer2D()->SetSkipBeginFrameRender(true);
		}

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

	void ImGuiEditorLayer::OnDetach(Application& app) {
		(void)app;
		if (m_LogSubscriptionId.value != 0) {
			Log::OnLog.Remove(m_LogSubscriptionId);
			m_LogSubscriptionId = EventId{};
		}

		DestroyFBO(m_EditorViewFBO);
		DestroyFBO(m_GameViewFBO);
		EditorIcons::Shutdown();
		m_AssetBrowser.Shutdown();
		m_PackageManagerPanel.Shutdown();
		m_PackageManager.Shutdown();
	}

	// ──────────────────────────────────────────────
	//  Dockspace & Menu
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::RenderDockspaceRoot() {
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

	void ImGuiEditorLayer::RenderMainMenu(Scene& scene) {
		if (!ImGui::BeginMenuBar()) {
			return;
		}


		if (ImGui::BeginMenu("Application")) {
			if (ImGui::MenuItem("Reload App")) {
				Application::Reload();
			}
			if (ImGui::MenuItem("Quit")) {
				Application::RequestQuit();
			}
			ImGui::EndMenu();
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
			ImGui::Separator();

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

			ExternalEditor::DetectEditors();
			const auto& editors = ExternalEditor::GetAvailableEditors();
			if (!editors.empty() && ImGui::BeginMenu("Script Editor")) {
				for (int i = 0; i < static_cast<int>(editors.size()); i++) {
					bool selected = (ExternalEditor::GetSelectedIndex() == i);
					const std::string editorId = std::to_string(i);
					if (ImGuiUtils::MenuItemEllipsis(editors[i].DisplayName, editorId.c_str(), nullptr, selected, true, 220.0f)) {
						ExternalEditor::SetSelectedIndex(i);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	static bool IconButton(const char* id, const char* iconName, float iconSize, const ImVec2& btnSize) {
		unsigned int tex = EditorIcons::Get(iconName, (int)iconSize);
		if (tex)
			return ImGui::ImageButton(id, (ImTextureID)(intptr_t)tex, btnSize, ImVec2(0, 1), ImVec2(1, 0));
		return ImGui::Button(id + 2);
	}

	void ImGuiEditorLayer::RenderToolbar() {
		ImGui::Begin("Toolbar");

		const float iconSize = ImGui::GetTextLineHeight();
		const ImVec2 btnSize(iconSize, iconSize);
		const bool isPlaying = Application::GetIsPlaying();
		const bool isPaused = Application::IsPlaymodePaused();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

		if (!isPlaying) {
			if (IconButton("##Play", "play", iconSize, btnSize)) {
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
				m_LogEntries.clear();
				Application::SetPlaymodePaused(false);
				Application::SetIsPlaying(true);

				if (active) {
					auto audioView = active->GetRegistry().view<AudioSourceComponent>(entt::exclude<DisabledTag>);
					for (auto [ent, audio] : audioView.each()) {
						if (audio.GetPlayOnAwake() && audio.GetAudioHandle().IsValid()) {
							audio.Play();
						}
					}
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play (Enter playmode)");
		}
		else {
			if (isPaused) {
				if (IconButton("##Continue", "play", iconSize, btnSize)) {
					Application::SetPlaymodePaused(false);
					Scene* active = SceneManager::Get().GetActiveScene();
					if (active) {
						for (auto ent : m_EditorPausedAudioEntities) {
							if (active->IsValid(ent) && active->HasComponent<AudioSourceComponent>(ent)) {
								auto& audio = active->GetComponent<AudioSourceComponent>(ent);
								if (audio.IsPaused()) audio.Resume();
							}
						}
					}
					m_EditorPausedAudioEntities.clear();
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Continue");
			}
			else {
				if (IconButton("##Pause", "pause", iconSize, btnSize)) {
					Application::SetPlaymodePaused(true);
					m_EditorPausedAudioEntities.clear();
					Scene* active = SceneManager::Get().GetActiveScene();
					if (active) {
						for (auto [ent, audio] : active->GetRegistry().view<AudioSourceComponent>().each()) {
							if (audio.IsPlaying()) {
								audio.Pause();
								m_EditorPausedAudioEntities.push_back(ent);
							}
						}
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause");
			}

			ImGui::SameLine();
			if (IconButton("##Step", "step_forward", iconSize, btnSize)) {
				Application::SetPlaymodePaused(false);
				m_StepFrames = 2;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step (advance one frame)");

			ImGui::SameLine();
			if (IconButton("##Stop", "stop", iconSize, btnSize)) {
				{
					Scene* act = SceneManager::Get().GetActiveScene();
					if (act) {
						for (auto [ent, audio] : act->GetRegistry().view<AudioSourceComponent>().each()) {
							if (audio.IsPlaying() || audio.IsPaused()) audio.Stop();
						}
					}
				}

				RestoreEditorSceneAfterPlaymode();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop (exit playmode)");
		}

		ImGui::PopStyleVar();
		ImGui::SameLine();
		const char* statusText = !isPlaying ? "Editor" : (isPaused ? "Paused" : "Playing");
		ImGui::TextUnformatted(statusText);
		ImGui::End();

		// Handle step-frame logic: pause again after stepping one frame
		if (isPlaying && m_StepFrames > 0) {
			m_StepFrames--;
			if (m_StepFrames == 0)
				Application::SetPlaymodePaused(true);
		}
	}

	void ImGuiEditorLayer::RestoreEditorSceneAfterPlaymode() {
		uint64_t selectedUUID = 0;
		Scene* active = SceneManager::Get().GetActiveScene();
		if (active && m_SelectedEntity != entt::null && active->IsValid(m_SelectedEntity)
			&& active->HasComponent<UUIDComponent>(m_SelectedEntity)) {
			selectedUUID = static_cast<uint64_t>(active->GetComponent<UUIDComponent>(m_SelectedEntity).Id);
		}

		m_SelectedEntity = entt::null;
		m_RenamingEntity = entt::null;
		m_EntityOrder.clear();

		Application::SetPlaymodePaused(false);
		Application::SetIsPlaying(false);

		if (!m_PlayModeScenePath.empty()) {
			active = SceneManager::Get().GetActiveScene();
			if (active) {
				SceneSerializer::LoadFromFile(*active, m_PlayModeScenePath);
			}
			m_PlayModeScenePath.clear();
		}

		if (selectedUUID == 0) {
			return;
		}

		active = SceneManager::Get().GetActiveScene();
		if (!active) {
			return;
		}

		auto view = active->GetRegistry().view<UUIDComponent>();
		for (auto [ent, uuid] : view.each()) {
			if (static_cast<uint64_t>(uuid.Id) == selectedUUID) {
				m_SelectedEntity = ent;
				break;
			}
		}
	}



	// ──────────────────────────────────────────────
	//  Entities Panel (with rename support)
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::RenderEntitiesPanel() {
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

			ImGui::PushID(static_cast<int>(static_cast<uint64_t>(scene.GetSceneId())));

			ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			const std::string fullSceneLabel = scene.IsDirty() ? scene.GetName() + " *" : scene.GetName();
			bool sceneLabelTruncated = false;
			const std::string sceneLabel = ImGuiUtils::Ellipsize(fullSceneLabel, ImGui::GetContentRegionAvail().x, &sceneLabelTruncated);
			bool sceneOpen = ImGui::TreeNodeEx(sceneLabel.c_str(), sceneFlags);
			if (sceneLabelTruncated && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", fullSceneLabel.c_str());
			}

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

				// Rebuild entity order if size changed (scene load, entity create/destroy)
				{
					std::vector<entt::entity> viewEntities(view.begin(), view.end());
					if (m_EntityOrder.size() != viewEntities.size()) {
						m_EntityOrder = viewEntities;
					} else {
						std::unordered_set<uint32_t> viewSet;
						for (auto e : viewEntities) viewSet.insert(static_cast<uint32_t>(e));
						m_EntityOrder.erase(
							std::remove_if(m_EntityOrder.begin(), m_EntityOrder.end(),
								[&](entt::entity e) { return viewSet.find(static_cast<uint32_t>(e)) == viewSet.end(); }),
							m_EntityOrder.end());
						std::unordered_set<uint32_t> orderSet;
						for (auto e : m_EntityOrder) orderSet.insert(static_cast<uint32_t>(e));
						for (auto e : viewEntities) {
							if (orderSet.find(static_cast<uint32_t>(e)) == orderSet.end())
								m_EntityOrder.push_back(e);
						}
					}
				}

				for (int entityIdx = 0; entityIdx < static_cast<int>(m_EntityOrder.size()); entityIdx++) {
					const EntityHandle entityHandle = m_EntityOrder[entityIdx];
					if (!scene.IsValid(entityHandle)) continue;
					Entity entity = scene.GetEntity(entityHandle);
					const bool selected = m_SelectedEntity == entityHandle;

					ImGui::PushID(static_cast<int>(static_cast<uint32_t>(entityHandle)));

					bool entityIsDisabled = scene.HasComponent<DisabledTag>(entityHandle);
					if (entityIsDisabled)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

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
						bool entityLabelTruncated = false;
						const std::string entityLabel = ImGuiUtils::Ellipsize(entity.GetName(), ImGui::GetContentRegionAvail().x, &entityLabelTruncated);
						if (ImGui::Selectable(entityLabel.c_str(), selected)) {
							m_SelectedEntity = entityHandle;
						}
						if (entityLabelTruncated && ImGui::IsItemHovered()) {
							ImGui::SetTooltip("%s", entity.GetName().c_str());
						}

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							m_RenamingEntity = entityHandle;
							m_EntityRenameFrameCounter = 0;
							std::snprintf(m_EntityRenameBuffer, sizeof(m_EntityRenameBuffer), "%s", entity.GetName().c_str());
						}
					}

					// Drag-drop source: drag entity to reorder or to asset browser for prefab
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
						struct HierarchyDragData { int Index; uint32_t EntityHandle; };
						HierarchyDragData dragData{ entityIdx, static_cast<uint32_t>(entityHandle) };
						ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &dragData, sizeof(dragData));
						ImGui::Text("Move: %s", entity.GetName().c_str());
						ImGui::EndDragDropSource();
					}

					// Drag-drop target: reorder entities, or accept .prefab from asset browser
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
							struct HierarchyDragData { int Index; uint32_t EntityHandle; };
							auto* dragData = static_cast<const HierarchyDragData*>(payload->Data);
							int srcIndex = dragData->Index;
							if (srcIndex != entityIdx && srcIndex >= 0 && srcIndex < static_cast<int>(m_EntityOrder.size())) {
								entt::entity moved = m_EntityOrder[srcIndex];
								m_EntityOrder.erase(m_EntityOrder.begin() + srcIndex);
								int insertAt = (srcIndex < entityIdx) ? entityIdx - 1 : entityIdx;
								m_EntityOrder.insert(m_EntityOrder.begin() + insertAt, moved);
							}
						}
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
							std::string droppedPath(static_cast<const char*>(payload->Data));
							std::string ext = std::filesystem::path(droppedPath).extension().string();
							std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
							if (ext == ".prefab") {
								SceneSerializer::LoadEntityFromFile(scene, droppedPath);
								m_EntityOrder.clear();
							}
						}
						ImGui::EndDragDropTarget();
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
							m_EntityOrder.clear();
						}

						if (ImGui::MenuItem("Duplicate"))
						{
							Entity clone = scene.CreateEntity(entity.GetName() + " (Clone)");

							const auto& compReg = SceneManager::Get().GetComponentRegistry();
							compReg.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
								if (info.category != ComponentCategory::Component) return;
								if (!info.has(entity)) return;
								if (info.copyTo)
									info.copyTo(entity, clone);
							});

							m_SelectedEntity = clone.GetHandle();
							scene.MarkDirty();
							m_EntityOrder.clear();
						}

						if (ImGui::MenuItem("Rename"))
						{
							m_RenamingEntity = entityHandle;
							m_EntityRenameFrameCounter = 0;
							std::snprintf(m_EntityRenameBuffer, sizeof(m_EntityRenameBuffer), "%s", entity.GetName().c_str());
						}

						ImGui::EndPopup();
					}

					if (entityIsDisabled)
						ImGui::PopStyleColor();

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
						else if (ext == ".prefab") {
							Scene* dropScene = SceneManager::Get().GetActiveScene();
							if (dropScene) {
								SceneSerializer::LoadEntityFromFile(*dropScene, droppedPath);
								m_EntityOrder.clear();
							}
						}
						else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
							Scene* dropScene = SceneManager::Get().GetActiveScene();
							if (dropScene) {
								std::string entityName = std::filesystem::path(droppedPath).stem().string();
								Entity newEntity = dropScene->CreateEntity(entityName);
								auto& sr = newEntity.AddComponent<SpriteRendererComponent>();
								sr.TextureHandle = TextureManager::LoadTexture(droppedPath);

								Texture2D* tex = TextureManager::GetTexture(sr.TextureHandle);
								if (tex && tex->IsValid() && tex->GetHeight() > 0) {
									float aspect = (float)tex->GetWidth() / (float)tex->GetHeight();
									auto& transform = newEntity.GetComponent<Transform2DComponent>();
									transform.Scale = { aspect, 1.0f };
								}

								m_SelectedEntity = newEntity.GetHandle();
								dropScene->MarkDirty();
							}
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !ImGui::IsAnyItemHovered()) {
			m_SelectedEntity = entt::null;
			m_RenamingEntity = entt::null;
		}

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Inspector Panel
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::RenderInspectorPanel(Scene& scene) {
		ImGui::Begin("Inspector");

		if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
			m_SelectedEntity = entt::null;
			RenderAssetInspector();
			ImGui::End();
			m_InspectorItemWasActive = false;
			return;
		}

		Entity entity = scene.GetEntity(m_SelectedEntity);

		// ── Entity Header: Name + Toggles + UUID ──────────────────

		// Editable Name
		std::string entityName = entity.HasComponent<NameComponent>()
			? entity.GetComponent<NameComponent>().Name : "Entity";
		char nameBuf[256];
		std::snprintf(nameBuf, sizeof(nameBuf), "%s", entityName.c_str());
		ImGui::SetNextItemWidth(-1);
		if (ImGui::InputText("##EntityName", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (entity.HasComponent<NameComponent>()) {
				entity.GetComponent<NameComponent>().Name = nameBuf;
			} else {
				scene.AddComponent<NameComponent>(m_SelectedEntity, std::string(nameBuf));
			}
			scene.MarkDirty();
		}

		// Toggles row: Enabled + Static
		{
			bool isDisabled = entity.HasComponent<DisabledTag>();
			bool isEnabled = !isDisabled;
			if (ImGui::Checkbox("Enabled", &isEnabled)) {
				if (isEnabled && entity.HasComponent<DisabledTag>())
					entity.RemoveComponent<DisabledTag>();
				else if (!isEnabled && !entity.HasComponent<DisabledTag>())
					entity.AddComponent<DisabledTag>();
				scene.MarkDirty();
			}

			ImGui::SameLine();

			bool isStatic = entity.HasComponent<StaticTag>();
			if (ImGui::Checkbox("Static", &isStatic)) {
				if (isStatic && !entity.HasComponent<StaticTag>())
					entity.AddComponent<StaticTag>();
				else if (!isStatic && entity.HasComponent<StaticTag>())
					entity.RemoveComponent<StaticTag>();
				scene.MarkDirty();
			}
		}

		// UUID display (read-only, small gray text)
		if (entity.HasComponent<UUIDComponent>()) {
			uint64_t uuid = static_cast<uint64_t>(entity.GetComponent<UUIDComponent>().Id);
			ImGui::TextDisabled("UUID: %llu", uuid);
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Click to copy");
			}
			if (ImGui::IsItemClicked()) {
				ImGui::SetClipboardText(std::to_string(uuid).c_str());
			}
		}

		ImGui::Separator();

		const auto& registry = SceneManager::Get().GetComponentRegistry();

		std::type_index pendingRemoval = typeid(void);

		registry.ForEachComponentInfo([&](const std::type_index& typeId, const ComponentInfo& info) {
			if (info.category != ComponentCategory::Component) return;
			if (info.displayName == "Name") return; // Shown in entity header
			if (!info.has(entity)) return;

			// Scripts render their own per-script sections — skip the outer wrapper
			if (info.displayName == "Scripts") {
				if (info.drawInspector) info.drawInspector(entity);
				return;
			}

			bool removeRequested = false;
			bool open = ImGuiUtils::BeginComponentSection(info.displayName.c_str(), removeRequested);

			// Component drag source on the header (attaches to the CollapsingHeader item)
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				uint64_t entityUUID = 0;
				if (entity.HasComponent<UUIDComponent>())
					entityUUID = static_cast<uint64_t>(entity.GetComponent<UUIDComponent>().Id);
				std::string refStr = std::to_string(entityUUID) + ":" + info.displayName;
				ImGui::SetDragDropPayload("COMPONENT_REF", refStr.c_str(), refStr.size() + 1);
				ImGui::Text("Ref: %s.%s", entity.GetName().c_str(), info.displayName.c_str());
				ImGui::EndDragDropSource();
			}

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

			bool hasFilter = !filter.empty();

			if (hasFilter) {
				// Flat filtered list when searching
				registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
					if (info.category != ComponentCategory::Component) return;
					if (info.has(entity)) return;

					std::string lowerName = info.displayName;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
					if (lowerName.find(filter) == std::string::npos) return;

					if (ImGuiUtils::MenuItemEllipsis(info.displayName, info.displayName.c_str(), nullptr, false, true, 260.0f)) {
						info.add(entity);
						scene.MarkDirty();
					}
				});

				// Also search scripts by name
				{
					std::vector<std::pair<std::string, std::string>> scriptFiles; // className, path
					BoltProject* project = ProjectManager::GetCurrentProject();
					if (project) {
						std::filesystem::path scriptsDir(project->ScriptsDirectory);
						if (std::filesystem::exists(scriptsDir)) {
							for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptsDir)) {
								if (!entry.is_regular_file()) continue;
								std::string ext = entry.path().extension().string();
								std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
								if (ext != ".cs") continue;
								std::string className = entry.path().stem().string();
								std::string lowerClassName = className;
								std::transform(lowerClassName.begin(), lowerClassName.end(), lowerClassName.begin(), ::tolower);
								if (lowerClassName.find(filter) != std::string::npos) {
									scriptFiles.emplace_back(className, entry.path().string());
								}
							}
						}
					}
					for (const auto& [className, path] : scriptFiles) {
						std::string label = className + "  .cs";
						if (ImGuiUtils::MenuItemEllipsis(label, path.c_str(), nullptr, false, true, 260.0f)) {
							if (!entity.HasComponent<ScriptComponent>()) {
								entity.AddComponent<ScriptComponent>();
							}
							auto& sc = entity.GetComponent<ScriptComponent>();
							if (!sc.HasScript(className)) {
								sc.AddScript(className);
							}
							scene.MarkDirty();
						}
					}
				}
			} else {
				// Categorized tree view
				// Collect components grouped by subcategory
				std::vector<std::pair<std::string, std::vector<const ComponentInfo*>>> categories;
				std::unordered_map<std::string, size_t> categoryIndex;

				// Define subcategory ordering
				const std::vector<std::string> subcategoryOrder = {
					"General", "Rendering", "Physics", "Audio", "Scripting"
				};
				for (const auto& sub : subcategoryOrder) {
					categoryIndex[sub] = categories.size();
					categories.emplace_back(sub, std::vector<const ComponentInfo*>{});
				}

				registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
					if (info.category != ComponentCategory::Component) return;
					if (info.has(entity)) return;

					std::string sub = info.subcategory.empty() ? "General" : info.subcategory;
					auto it = categoryIndex.find(sub);
					if (it == categoryIndex.end()) {
						categoryIndex[sub] = categories.size();
						categories.emplace_back(sub, std::vector<const ComponentInfo*>{});
						it = categoryIndex.find(sub);
					}
					categories[it->second].second.push_back(&info);
				});

				for (const auto& [subcategory, components] : categories) {
					if (components.empty() && subcategory != "Scripting") continue;

					if (ImGui::TreeNode(subcategory.c_str())) {
						for (const auto* info : components) {
							if (ImGuiUtils::MenuItemEllipsis(info->displayName, info->displayName.c_str(), nullptr, false, true, 260.0f)) {
								info->add(entity);
								scene.MarkDirty();
							}
						}
						ImGui::TreePop();
					}
				}

				// Scripts subcategory: individual .cs files as addable items
				{
					std::vector<std::pair<std::string, std::string>> scriptFiles; // className, path
					BoltProject* project = ProjectManager::GetCurrentProject();
					if (project) {
						std::filesystem::path scriptsDir(project->ScriptsDirectory);
						if (std::filesystem::exists(scriptsDir)) {
							for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptsDir)) {
								if (!entry.is_regular_file()) continue;
								std::string ext = entry.path().extension().string();
								std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
								if (ext != ".cs") continue;
								scriptFiles.emplace_back(entry.path().stem().string(), entry.path().string());
							}
						}
					}
					std::sort(scriptFiles.begin(), scriptFiles.end());

					if (!scriptFiles.empty()) {
						if (ImGui::TreeNode("Scripts")) {
							for (const auto& [className, path] : scriptFiles) {
								if (ImGuiUtils::MenuItemEllipsis(className, path.c_str(), nullptr, false, true, 260.0f)) {
									if (!entity.HasComponent<ScriptComponent>()) {
										entity.AddComponent<ScriptComponent>();
									}
									auto& sc = entity.GetComponent<ScriptComponent>();
									if (!sc.HasScript(className)) {
										sc.AddScript(className);
									}
									scene.MarkDirty();
								}
							}
							ImGui::TreePop();
						}
					}
				}
			}

			ImGui::EndPopup();
		}

		// Drag-drop target: accept .cs files dropped onto the Inspector panel
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				std::string droppedPath(static_cast<const char*>(payload->Data));
				std::string ext = std::filesystem::path(droppedPath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

				if (ext == ".cs") {
					std::string className = std::filesystem::path(droppedPath).stem().string();
					if (!entity.HasComponent<ScriptComponent>()) {
						entity.AddComponent<ScriptComponent>();
					}
					auto& sc = entity.GetComponent<ScriptComponent>();
					if (!sc.HasScript(className)) {
						sc.AddScript(className);
					}
					scene.MarkDirty();
				}
			}
			ImGui::EndDragDropTarget();
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

	void ImGuiEditorLayer::RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
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

	void ImGuiEditorLayer::RenderEditorView(Scene& scene) {
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
				Gizmo::SetViewportAABBOverride(viewAABB);

				// Draw selection outline for selected entity
				if (m_SelectedEntity != entt::null && scene.IsValid(m_SelectedEntity)
					&& scene.HasComponent<Transform2DComponent>(m_SelectedEntity)) {

					auto& transform = scene.GetComponent<Transform2DComponent>(m_SelectedEntity);
					Vec2 outlineScale = transform.Scale;

					if (scene.HasComponent<SpriteRendererComponent>(m_SelectedEntity)) {
						auto& sr = scene.GetComponent<SpriteRendererComponent>(m_SelectedEntity);
						Texture2D* tex = TextureManager::GetTexture(sr.TextureHandle);
						if (tex && tex->IsValid() && tex->GetHeight() > 0) {
							// bounds already encoded in transform scale
						}
					}

					Color prevColor = Gizmo::GetColor();
					float prevWidth = Gizmo::GetLineWidth();
					Gizmo::SetColor(Color(1.0f, 0.6f, 0.0f, 1.0f)); // Orange
					Gizmo::SetLineWidth(2.0f);
					Gizmo::DrawSquare(transform.Position, outlineScale, transform.Rotation * (180.0f / 3.14159265f));
					Gizmo::SetColor(prevColor);
					Gizmo::SetLineWidth(prevWidth);
				}

				// Editor view always uses a stable default background
				static const Color k_EditorClearColor(0.18f, 0.18f, 0.20f, 1.0f);
				RenderSceneIntoFBO(m_EditorViewFBO, scene, vp, viewAABB, true, k_EditorClearColor);
				Gizmo::ClearViewportAABBOverride();

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_EditorViewFBO.ColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));

				ImVec2 imageTopLeft = ImGui::GetItemRectMin();

				// Overlay component icons at entity positions
				{
					const float iconSize = 24.0f;
					const float halfIcon = iconSize * 0.5f;

					auto camView = scene.GetRegistry().view<Camera2DComponent, Transform2DComponent>();
					for (auto [ent, cam, transform] : camView.each()) {
						glm::vec4 worldPos(transform.Position.x, transform.Position.y, 0.0f, 1.0f);
						glm::vec4 clipPos = vp * worldPos;

						// Perspective divide (orthographic, w should be 1)
						if (clipPos.w == 0.0f) continue;
						float ndcX = clipPos.x / clipPos.w;
						float ndcY = clipPos.y / clipPos.w;

						// NDC to screen: [-1,1] -> [0, viewportSize]
						float screenX = (ndcX * 0.5f + 0.5f) * viewportSize.x;
						float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * viewportSize.y;

						// Skip if outside visible viewport
						if (screenX < -halfIcon || screenX > viewportSize.x + halfIcon ||
							screenY < -halfIcon || screenY > viewportSize.y + halfIcon)
							continue;

						unsigned int camIcon = EditorIcons::Get("camera", 24);
						if (camIcon) {
							ImVec2 iconPos(imageTopLeft.x + screenX - halfIcon,
							               imageTopLeft.y + screenY - halfIcon);
							ImGui::GetWindowDrawList()->AddImage(
								static_cast<ImTextureID>(static_cast<intptr_t>(camIcon)),
								iconPos,
								ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
								ImVec2(0, 1), ImVec2(1, 0));
						}
					}
				}
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

	void ImGuiEditorLayer::RenderGameView(Scene& scene) {
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

	void ImGuiEditorLayer::RenderLogPanel() {
		ImGui::Begin("Log");

		// Toolbar: Clear + Level filters
		if (ImGui::Button("Clear")) {
			m_LogEntries.clear();
		}

		// Count messages per level for display
		int infoCount = 0, warnCount = 0, errorCount = 0;
		for (const auto& e : m_LogEntries) {
			if (e.Level <= Log::Level::Info) infoCount++;
			else if (e.Level == Log::Level::Warn) warnCount++;
			else errorCount++;
		}

		ImGui::SameLine();
		ImGui::TextDisabled("|");
		ImGui::SameLine();

		// Info filter toggle
		{
			unsigned int infoIcon = EditorIcons::Get("info", 16);
			if (infoIcon) {
				ImVec4 tint = m_ShowLogInfo ? ImVec4(1,1,1,1) : ImVec4(0.4f,0.4f,0.4f,0.5f);
				if (ImGui::ImageButton("##FilterInfo",
					static_cast<ImTextureID>(static_cast<intptr_t>(infoIcon)),
					ImVec2(14, 14), ImVec2(0,1), ImVec2(1,0), ImVec4(0,0,0,0), tint))
					m_ShowLogInfo = !m_ShowLogInfo;
			} else {
				if (ImGui::SmallButton(m_ShowLogInfo ? "[I]" : "( )"))
					m_ShowLogInfo = !m_ShowLogInfo;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Info (%d)", infoCount);
		}

		ImGui::SameLine();

		// Warn filter toggle
		{
			unsigned int warnIcon = EditorIcons::Get("warning", 16);
			ImVec4 tint = m_ShowLogWarn ? ImVec4(1,1,1,1) : ImVec4(0.4f,0.4f,0.4f,0.5f);
			if (warnIcon) {
				if (ImGui::ImageButton("##FilterWarn",
					static_cast<ImTextureID>(static_cast<intptr_t>(warnIcon)),
					ImVec2(14, 14), ImVec2(0,1), ImVec2(1,0), ImVec4(0,0,0,0), tint))
					m_ShowLogWarn = !m_ShowLogWarn;
			} else {
				ImGui::PushStyleColor(ImGuiCol_Text, tint);
				if (ImGui::SmallButton(m_ShowLogWarn ? "W" : "(W)"))
					m_ShowLogWarn = !m_ShowLogWarn;
				ImGui::PopStyleColor();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Warnings (%d)", warnCount);
		}

		ImGui::SameLine();

		// Error filter toggle
		{
			unsigned int errIcon = EditorIcons::Get("error", 16);
			ImVec4 tint = m_ShowLogError ? ImVec4(1,1,1,1) : ImVec4(0.4f,0.4f,0.4f,0.5f);
			if (errIcon) {
				if (ImGui::ImageButton("##FilterError",
					static_cast<ImTextureID>(static_cast<intptr_t>(errIcon)),
					ImVec2(14, 14), ImVec2(0,1), ImVec2(1,0), ImVec4(0,0,0,0), tint))
					m_ShowLogError = !m_ShowLogError;
			} else {
				ImGui::PushStyleColor(ImGuiCol_Text, tint);
				if (ImGui::SmallButton(m_ShowLogError ? "E" : "(E)"))
					m_ShowLogError = !m_ShowLogError;
				ImGui::PopStyleColor();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Errors (%d)", errorCount);
		}

		ImGui::Separator();

		// Log entries with filtering and right-click copy
		ImGui::BeginChild("LogScroll");
		const bool stickToBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImGuiStyle& style = ImGui::GetStyle();
		for (int i = 0; i < static_cast<int>(m_LogEntries.size()); i++) {
			const LogEntry& entry = m_LogEntries[i];

			// Level filtering
			if (entry.Level <= Log::Level::Info && !m_ShowLogInfo) continue;
			if (entry.Level == Log::Level::Warn && !m_ShowLogWarn) continue;
			if (entry.Level >= Log::Level::Error && !m_ShowLogError) continue;

			// Level color
			ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
			if (entry.Level == Log::Level::Warn)
				color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
			else if (entry.Level >= Log::Level::Error)
				color = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);

			ImGui::PushID(i);
			const float rowWidth = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
			const float wrapWidth = std::max(rowWidth - style.FramePadding.x * 2.0f, 1.0f);
			const ImVec2 textSize = ImGui::CalcTextSize(entry.Message.c_str(), nullptr, false, wrapWidth);
			const float rowHeight = std::max(textSize.y, ImGui::GetTextLineHeight()) + style.FramePadding.y * 2.0f;
			const ImVec2 rowMin = ImGui::GetCursorScreenPos();
			const ImVec2 rowMax(rowMin.x + rowWidth, rowMin.y + rowHeight);

			ImGui::InvisibleButton("##LogEntry", ImVec2(rowWidth, rowHeight));
			if (ImGui::IsItemHovered()) {
				drawList->AddRectFilled(rowMin, rowMax, IM_COL32(70, 78, 92, 120), 4.0f);
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				ImGui::SetClipboardText(entry.Message.c_str());
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Click to copy");
			}

			// Right-click context menu
			if (ImGui::BeginPopupContextItem("##LogCtx")) {
				if (ImGui::MenuItem("Copy")) {
					ImGui::SetClipboardText(entry.Message.c_str());
				}
				if (ImGui::MenuItem("Copy All Visible")) {
					std::string all;
					for (const auto& e : m_LogEntries) {
						if (e.Level <= Log::Level::Info && !m_ShowLogInfo) continue;
						if (e.Level == Log::Level::Warn && !m_ShowLogWarn) continue;
						if (e.Level >= Log::Level::Error && !m_ShowLogError) continue;
						all += e.Message + "\n";
					}
					ImGui::SetClipboardText(all.c_str());
				}
				ImGui::EndPopup();
			}

			ImGui::SetCursorScreenPos(ImVec2(rowMin.x + style.FramePadding.x, rowMin.y + style.FramePadding.y));
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::PushTextWrapPos(rowMin.x + rowWidth - style.FramePadding.x);
			ImGui::TextUnformatted(entry.Message.c_str());
			ImGui::PopTextWrapPos();
			ImGui::PopStyleColor();
			ImGui::SetCursorScreenPos(ImVec2(rowMin.x, rowMin.y + rowHeight));

			ImGui::PopID();
		}

		if (stickToBottom) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Project Panel (Asset Browser)
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::RenderProjectPanel() {
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
		if (m_AssetBrowser.TakeSelectionActivated() && !m_AssetBrowser.GetSelectedPath().empty()) {
			m_SelectedEntity = entt::null;
			m_RenamingEntity = entt::null;
		}
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

	void ImGuiEditorLayer::ExecuteBuild() {
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
			Process::Result buildResult = Process::Run({
				"dotnet",
				"build",
				project->CsprojPath,
				"-c", "Release",
				"--nologo",
				"-v", "q",
				"-p:DefineConstants=BOLT_BUILD%3BBT_RELEASE"
			});
			if (!buildResult.Succeeded()) {
				BT_ERROR_TAG("Build", "C# script compilation failed (exit code {}).", buildResult.ExitCode);
				if (!buildResult.Output.empty()) {
					BT_ERROR_TAG("Build", "{}", buildResult.Output);
				}
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
		// Try project-local BoltAssets first, fall back to engine's BoltAssets
		{
			std::string boltAssetsSrc;
			if (std::filesystem::exists(project->BoltAssetsDirectory)) {
				boltAssetsSrc = project->BoltAssetsDirectory;
			} else {
				boltAssetsSrc = Path::ResolveBoltAssets("");
			}

			if (!boltAssetsSrc.empty() && std::filesystem::exists(boltAssetsSrc)) {
				int n = CopyDirIncremental(boltAssetsSrc, outDir / "BoltAssets");
				BT_INFO_TAG("Build", "BoltAssets: {} file(s) updated", n);
			} else {
				BT_WARN_TAG("Build", "BoltAssets not found — build may be incomplete");
			}
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

#ifdef BT_PLATFORM_WINDOWS
		ShellExecuteA(nullptr, "open", outDir.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
	}

	void ImGuiEditorLayer::RenderBuildPanel() {
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
						const std::string sceneItemId = std::to_string(i);
						ImGuiUtils::SelectableEllipsis(m_BuildSceneList[i], sceneItemId.c_str());

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
									int insertAt = (srcIndex < i) ? i - 1 : i;
									m_BuildSceneList.insert(m_BuildSceneList.begin() + insertAt, moved);
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

		float halfWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Build", ImVec2(halfWidth, 0))) {
			m_BuildState = 1;
			m_BuildAndPlay = false;
			m_BuildStartTime = std::chrono::steady_clock::now();
		}
		ImGui::SameLine();
		if (ImGui::Button("Build and Play", ImVec2(-1, 0))) {
			m_BuildState = 1;
			m_BuildAndPlay = true;
			m_BuildStartTime = std::chrono::steady_clock::now();
		}

		if (!canBuild) ImGui::EndDisabled();

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Player Settings (separate window)
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::RenderPlayerSettingsPanel() {
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

		if (ImGui::CollapsingHeader("App Icon", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(8);

			if (!project->AppIconPath.empty()) {
				TextureHandle iconHandle = TextureManager::LoadTexture(project->AppIconPath);
				Texture2D* iconTex = TextureManager::GetTexture(iconHandle);
				if (iconTex && iconTex->IsValid()) {
					ImGui::Image(
						static_cast<ImTextureID>(static_cast<intptr_t>(iconTex->GetHandle())),
						ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::SameLine();
				}
				else {
					ImGui::TextDisabled("Failed to load:");
					ImGuiUtils::TextDisabledEllipsis(project->AppIconPath);
				}

				if (ImGui::Button("Clear")) {
					project->AppIconPath.clear();
					changed = true;
					Application::GetInstance()->GetWindow()->SetWindowIconFromResource();
				}

				if (iconTex && iconTex->IsValid()) {
					ImGuiUtils::TextDisabledEllipsis(project->AppIconPath);
				}
			} else {
				ImGui::TextDisabled("No icon set");
				ImGui::TextDisabled("Drag an image from the Asset Browser");
			}

			// Drop target for asset browser images
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
					std::string droppedPath(static_cast<const char*>(payload->Data));
					std::string ext = std::filesystem::path(droppedPath).extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

					if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
						// Store relative path from Assets directory
						std::filesystem::path absPath(droppedPath);
						std::filesystem::path assetsDir(project->AssetsDirectory);
						if (absPath.string().find(assetsDir.string()) == 0) {
							project->AppIconPath = std::filesystem::relative(absPath, assetsDir.parent_path()).string();
						} else {
							project->AppIconPath = absPath.filename().string();
						}
						changed = true;

						// Apply icon to editor window immediately
						TextureHandle h = TextureManager::LoadTexture(project->AppIconPath);
						Texture2D* tex = TextureManager::GetTexture(h);
						if (tex && tex->IsValid()) {
							Application::GetInstance()->GetWindow()->SetWindowIcon(tex);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Unindent(8);
		}

		if (changed) project->Save();

		ImGui::End();
	}

	// ──────────────────────────────────────────────
	//  Main OnGui entry point
	// ──────────────────────────────────────────────

	void ImGuiEditorLayer::OnImGuiRender(Application& app) {
		(void)app;
		Scene* activeScene = SceneManager::Get().GetActiveScene();
		if (!activeScene) {
			return;
		}
		Scene& scene = *activeScene;

		// Deferred build execution: state 1 -> 2 lets the overlay render one frame first
		if (m_BuildState == 1) {
			m_BuildState = 2;
		} else if (m_BuildState == 2) {
			ExecuteBuild();
			m_BuildState = 0;

			// Launch the built game if "Build and Play" was clicked
			if (m_BuildAndPlay) {
				m_BuildAndPlay = false;
				BoltProject* project = ProjectManager::GetCurrentProject();
				if (project) {
					std::string outputDir = m_BuildOutputDirBuffer;
					if (outputDir.empty())
						outputDir = Path::Combine(project->RootDirectory, "Builds");
#ifdef BT_PLATFORM_WINDOWS
					auto exePath = std::filesystem::path(outputDir) / (project->Name + ".exe");
					if (std::filesystem::exists(exePath)) {
						std::string cmd = "\"" + std::filesystem::canonical(exePath).string() + "\"";
						std::wstring wcmd(cmd.begin(), cmd.end());
						std::vector<wchar_t> buf(wcmd.begin(), wcmd.end());
						buf.push_back(L'\0');
						STARTUPINFOW si{}; si.cb = sizeof(si);
						PROCESS_INFORMATION pi{};
						CreateProcessW(nullptr, buf.data(), nullptr, nullptr,
							FALSE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi);
						CloseHandle(pi.hProcess);
						CloseHandle(pi.hThread);
						BT_INFO_TAG("Build", "Launched: {}", exePath.string());
					} else {
						BT_CORE_ERROR_TAG("Build", "Built executable not found: {}", exePath.string());
					}
#endif
				}
			}
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
				if (auto loaded = weakScene.lock()) {
					SceneSerializer::LoadFromFile(*loaded, dropPath);
					m_EntityOrder.clear();
				}
			}
		}

		// Process deferred scene switch (blocked during Play Mode)
		if (!m_PendingSceneSwitch.empty() && !Application::GetIsPlaying()) {
			std::string switchPath = m_PendingSceneSwitch;
			m_PendingSceneSwitch.clear();

			Scene* active = SceneManager::Get().GetActiveScene();
			if (active) {
				SceneSerializer::LoadFromFile(*active, switchPath);
				m_EntityOrder.clear();
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

		// Intercept quit request: exit playmode first, then check for unsaved changes
		if (Application::IsQuitRequested()) {
			// Exit playmode and restore scene first
			if (Application::GetIsPlaying()) {
				RestoreEditorSceneAfterPlaymode();
			}

			Scene* active = SceneManager::Get().GetActiveScene();
			if (active && active->IsDirty()) {
				m_ShowQuitSaveDialog = true;
				Application::CancelQuit();
			} else {
				Application::ConfirmQuit();
			}
		}

		RenderDockspaceRoot();
		RenderMainMenu(scene);

		// Save confirmation modal dialog (scene switch)
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

		// Quit confirmation modal dialog
		if (m_ShowQuitSaveDialog) {
			ImGui::OpenPopup("Save Before Quit?");
			m_ShowQuitSaveDialog = false;
		}
		if (ImGui::BeginPopupModal("Save Before Quit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			Scene* active = SceneManager::Get().GetActiveScene();
			std::string activeName = active ? active->GetName() : "Scene";
			ImGui::Text("Save changes to %s before closing?", activeName.c_str());
			ImGui::Spacing();

			if (ImGui::Button("Save", ImVec2(100, 0))) {
				if (active) {
					BoltProject* project = ProjectManager::GetCurrentProject();
					if (project) {
						std::string savePath = project->GetSceneFilePath(active->GetName());
						SceneSerializer::SaveToFile(*active, savePath);
						project->LastOpenedScene = active->GetName();
						project->Save();
					}
				}
				ImGui::CloseCurrentPopup();
				Application::ConfirmQuit();
			}
			ImGui::SameLine();
			if (ImGui::Button("Don't Save", ImVec2(100, 0))) {
				ImGui::CloseCurrentPopup();
				Application::ConfirmQuit();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// Suppress script recompilation while a script is being created/renamed
		if (scene.HasSystem<ScriptSystem>()) {
			auto* scriptSys = scene.GetSystem<ScriptSystem>();
				if (scriptSys)
					scriptSys->SetRecompileSuppressed(m_AssetBrowser.IsCreatingScript());
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

	void ImGuiEditorLayer::RenderAssetInspector() {
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

		ImGui::TextDisabled("Asset:");
		ImGui::SameLine();
		ImGuiUtils::TextEllipsis(name);
		ImGui::Separator();

		// File info
		ImGui::TextDisabled("Path:");
		ImGui::SameLine();
		ImGuiUtils::TextDisabledEllipsis(selectedPath);

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

	void ImGuiEditorLayer::RenderPackageManagerPanel() {
		if (!m_ShowPackageManager) return;

		if (!m_PackageManagerInitialized) {
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto toolPath = exeDir / ".." / ".." / ".." / "Bolt-PackageTool" / "bin" / "Release" / "net9.0" / "Bolt-PackageTool.exe";
			m_PackageManager.Initialize(toolPath.string());

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
