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
#include <Scene/EntityHelper.hpp>

#include "Graphics/TextureManager.hpp"

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
			if (ImGui::MenuItem("Create Square"))
			{
				Entity created = scene.CreateEntity("Square " + std::to_string(EntityHelper::EntitiesCount()));
				created.AddComponent<SpriteRendererComponent>();
				m_SelectedEntity = created.GetHandle();
			}
			if (ImGui::MenuItem("Create Circle"))
			{
				Entity created = scene.CreateEntity("Circle " + std::to_string(EntityHelper::EntitiesCount()));
				auto& sprite = created.AddComponent<SpriteRendererComponent>();
				sprite.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::Circle);
				m_SelectedEntity = created.GetHandle();
			}
			if (ImGui::MenuItem("Create Physics Entity"))
			{
				Entity created = scene.CreateEntity("Physics " + std::to_string(EntityHelper::EntitiesCount()));
				created.AddComponent<SpriteRendererComponent>();
				created.AddComponent<Rigidbody2DComponent>();
				created.AddComponent<BoxCollider2DComponent>();
				m_SelectedEntity = created.GetHandle();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Audio"))
			{
				if (ImGui::MenuItem("Create Audio Source"))
				{
					Entity created = scene.CreateEntity("AudioSource " + std::to_string(EntityHelper::EntitiesCount()));
					created.AddComponent<AudioSourceComponent>();
					m_SelectedEntity = created.GetHandle();
				}
				ImGui::EndMenu();
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

					if (entity.HasComponent<SpriteRendererComponent>())
						copy.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());
					if (entity.HasComponent<Rigidbody2DComponent>())
						copy.AddComponent<Rigidbody2DComponent>(entity.GetComponent<Rigidbody2DComponent>());
					if (entity.HasComponent<BoxCollider2DComponent>())
						copy.AddComponent<BoxCollider2DComponent>(entity.GetComponent<BoxCollider2DComponent>());

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

		if (entity.HasComponent<NameComponent>()) {
			auto& name = entity.GetComponent<NameComponent>();
			char buffer[256]{};
			std::snprintf(buffer, sizeof(buffer), "%s", name.Name.c_str());
			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				name.Name = buffer;
			}
		}

		if (entity.HasComponent<Transform2DComponent>()) {
			auto& transform = entity.GetComponent<Transform2DComponent>();
			ImGui::SeparatorText("Transform2D");
			ImGui::DragFloat2("Position", &transform.Position.x, 0.05f);
			ImGui::DragFloat2("Scale", &transform.Scale.x, 0.05f, 0.001f);
			ImGui::DragFloat("Rotation", &transform.Rotation, 0.01f);
		}

		if (entity.HasComponent<Rigidbody2DComponent>()) {
			auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
			ImGui::SeparatorText("Rigidbody2D");

			Vec2 position = rb2D.GetPosition();
			Vec2 velocity = rb2D.GetVelocity();
			float gravityScale = rb2D.GetGravityScale();
			float rotation = rb2D.GetRotation();

			if (ImGui::DragFloat2("Position##RigidBody2D", &position.x, 0.05f))
				rb2D.SetPosition(position);

			if (ImGui::DragFloat2("Velocity##RigidBody2D", &velocity.x, 0.05f))
				rb2D.SetVelocity(velocity);

			if (ImGui::DragFloat("Rotation##RigidBody2D", &rotation, 0.01f))
				rb2D.SetRotation(rotation);

			if (ImGui::SliderFloat("Gravity Scale", &gravityScale, 0.0f, 1.0f));
			rb2D.SetGravityScale(gravityScale);

			const char* items[] = { "Static", "Kinematic", "Dynamic" };
			static int currentItem = static_cast<int>(rb2D.GetBodyType());

			if (ImGui::BeginCombo("Body Type", items[currentItem])) {
				for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
					bool isSelected = (currentItem == i);

					if (ImGui::Selectable(items[i], isSelected)) {
						currentItem = i;
						rb2D.SetBodyType(static_cast<BodyType>(currentItem));
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}

		if (entity.HasComponent<SpriteRendererComponent>()) {
			auto& sprite = entity.GetComponent<SpriteRendererComponent>();
			ImGui::SeparatorText("Sprite Renderer");
			ImGui::ColorEdit4("Color", &sprite.Color.r, ImGuiColorEditFlags_NoInputs);
			ImGui::DragScalar("Sorting Order", ImGuiDataType_S16, &sprite.SortingOrder, 1.0f);
			ImGui::DragScalar("Sorting Layer", ImGuiDataType_U8, &sprite.SortingLayer, 1.0f);
		}

		if (entity.HasComponent<Camera2DComponent>()) {
			auto& camera = entity.GetComponent<Camera2DComponent>();
			ImGui::SeparatorText("Camera2D");
			float zoom = camera.GetZoom();
			if (ImGui::DragFloat("Zoom", &zoom, 0.01f, 0.01f, 100.0f)) {
				camera.SetZoom(zoom);
			}
			float ortho = camera.GetOrthographicSize();
			if (ImGui::DragFloat("Orthographic Size", &ortho, 0.05f, 0.05f, 1000.0f)) {
				camera.SetOrthographicSize(ortho);
			}
		}

		if (ImGui::Button("Delete Entity")) {
			scene.DestroyEntity(m_SelectedEntity);
			m_SelectedEntity = entt::null;
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
				auto* window = app->GetWindow();

				const int windowWidth = window->GetWidth();
				const int windowHeight = window->GetHeight();

				glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFramebufferId);
				glViewport(0, 0, framebufferWidth, framebufferHeight);

				Window::GetMainViewport()->SetSize(framebufferWidth, framebufferHeight);
				renderer->BeginFrame();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, windowWidth, windowHeight);
				Window::GetMainViewport()->SetSize(windowWidth, windowHeight);

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
		ImGui::TextDisabled("Input capture: %s", (m_IsViewportFocused || m_IsViewportHovered) ? "Viewport" : "Editor");

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

	void ImGuiEditorSystem::OnGui(Scene& scene) {
		RenderDockspaceRoot();
		RenderMainMenu(scene);
		RenderToolbar();
		RenderEntitiesPanel(scene);
		RenderInspectorPanel(scene);
		RenderViewportPanel(scene);
		RenderLogPanel();
		ImGui::End();
	}
}