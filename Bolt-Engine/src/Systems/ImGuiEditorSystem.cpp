#include <pch.hpp>
#include "ImGuiEditorSystem.hpp"

#include <imgui.h>

#include <cstdio>

#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Graphics/OpenGL.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/ComponentRegistry.hpp"
#include <Scene/EntityHelper.hpp>

#include "Graphics/TextureManager.hpp"
#include "Gui/ImGuiUtils.hpp"
#include "Serialization/Path.hpp"

namespace Bolt {
	void ImGuiEditorSystem::Awake(Scene& scene) {
		Application::SetIsPlaying(false);

		(void)scene;
		if (m_LogSubscriptionId.value != 0) {
			Log::OnLog.Remove(m_LogSubscriptionId);
		}

		m_LogEntries.clear();
		m_LogSubscriptionId = Log::OnLog.Add([this](const Log::Entry& entry) {
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

		DestroyViewportFramebuffer();
		m_AssetBrowser.Shutdown();
	}

	void ImGuiEditorSystem::EnsureViewportFramebuffer(int width, int height) {
		if (width <= 0 || height <= 0) {
			return;
		}

		const bool sizeChanged = m_EditorViewport.GetWidth() != width || m_EditorViewport.GetHeight() != height;
		if (m_ViewportFramebufferId != 0 && !sizeChanged) {
			return;
		}

		DestroyViewportFramebuffer();
		m_EditorViewport.SetSize(width, height);

		glGenFramebuffers(1, &m_ViewportFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFramebufferId);

		glGenTextures(1, &m_ViewportColorTextureId);
		glBindTexture(GL_TEXTURE_2D, m_ViewportColorTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportColorTextureId, 0);

		glGenRenderbuffers(1, &m_ViewportDepthRenderBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportDepthRenderBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportDepthRenderBufferId);

		BT_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, BoltErrorCode::InvalidHandle,
			"Editor viewport framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ImGuiEditorSystem::DestroyViewportFramebuffer() {
		if (m_ViewportDepthRenderBufferId != 0) {
			glDeleteRenderbuffers(1, &m_ViewportDepthRenderBufferId);
			m_ViewportDepthRenderBufferId = 0;
		}
		if (m_ViewportColorTextureId != 0) {
			glDeleteTextures(1, &m_ViewportColorTextureId);
			m_ViewportColorTextureId = 0;
		}
		if (m_ViewportFramebufferId != 0) {
			glDeleteFramebuffers(1, &m_ViewportFramebufferId);
			m_ViewportFramebufferId = 0;
		}
	}

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
			if (ImGui::MenuItem("Reload Scene")) {
				SceneManager::Get().ReloadScene(scene.GetName());
			}
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

		auto view = scene.GetRegistry().view<entt::entity>();
		for (const EntityHandle entityHandle : view) {
			Entity entity = scene.GetEntity(entityHandle);
			const bool selected = m_SelectedEntity == entityHandle;

			if (ImGui::Selectable(entity.GetName().c_str(), selected)) {
				m_SelectedEntity = entityHandle;
			}

			if (ImGui::BeginPopupContextItem())
			{
				m_SelectedEntity = entityHandle;

				if (ImGui::MenuItem("Delete Entity"))
				{
					scene.DestroyEntity(entity);
					if (m_SelectedEntity == entityHandle)
						m_SelectedEntity = entt::null;
				}

				if (ImGui::MenuItem("Copy Entity"))
				{
					Entity copy = scene.CreateEntity(entity.GetName() + " (Copy)");

					const auto& registry = SceneManager::Get().GetComponentRegistry();
					registry.ForEachComponentInfo([&](const std::type_index&, const ComponentInfo& info) {
						if (info.category != ComponentCategory::Component) return;
						if (!info.has(entity)) return;
						if (info.has(copy)) return; // already present (e.g. NameComponent, Transform2D)
						info.add(copy);
					});

					m_SelectedEntity = copy.GetHandle();
				}

				if (ImGui::MenuItem("Rename"))
				{

				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

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

		// Draw all components present on this entity via the registry
		std::type_index pendingRemoval = typeid(void);

		registry.ForEachComponentInfo([&](const std::type_index& typeId, const ComponentInfo& info) {
			if (info.category != ComponentCategory::Component) return;
			if (!info.has(entity)) return;

			bool removeRequested = false;
			ImGuiUtils::BeginComponentSection(info.displayName.c_str(), removeRequested);

			if (removeRequested) {
				pendingRemoval = typeId;
			}

			if (info.drawInspector) {
				info.drawInspector(entity);
			}
		});

		// Process deferred removal
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

		// "Add Component" button with dropdown
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

	void ImGuiEditorSystem::RenderViewportPanel(Scene& scene) {
		(void)scene;
		ImGui::Begin("Viewport");

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		const int framebufferWidth = static_cast<int>(viewportSize.x);
		const int framebufferHeight = static_cast<int>(viewportSize.y);

		if (framebufferWidth > 0 && framebufferHeight > 0) {
			EnsureViewportFramebuffer(framebufferWidth, framebufferHeight);

			if (m_ViewportFramebufferId != 0) {
				auto* app = Application::GetInstance();
				auto* renderer = app->GetRenderer2D();

				renderer->SetOutputTarget(m_ViewportFramebufferId, framebufferWidth, framebufferHeight);

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_ViewportColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));
			}
		}
		else {
			ImGui::TextDisabled("Viewport has no drawable area");
		}

		m_IsViewportHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
		m_IsViewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		ImGui::End();
	}

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

	void ImGuiEditorSystem::RenderProjectPanel() {
		if (!m_AssetBrowserInitialized) {
			std::string assetsRoot = Path::Combine(Path::ExecutableDir(), "Assets");
			if (!Directory::Exists(assetsRoot)) {
				// Fallback to working directory
				assetsRoot = Path::Combine(Path::Current(), "Assets");
			}
			if (!Directory::Exists(assetsRoot)) {
				// Last resort: create it
				Directory::Create(assetsRoot);
			}
			m_AssetBrowser.Initialize(assetsRoot);
			m_AssetBrowserInitialized = true;
		}

		m_AssetBrowser.Render();
	}

	void ImGuiEditorSystem::OnGui(Scene& scene) {
		RenderDockspaceRoot();
		RenderMainMenu(scene);
		RenderToolbar();
		RenderEntitiesPanel(scene);
		RenderInspectorPanel(scene);
		RenderViewportPanel(scene);
		RenderProjectPanel();
		RenderLogPanel();
		ImGui::End();
	}
}
