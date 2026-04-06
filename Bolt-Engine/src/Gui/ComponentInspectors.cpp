#include <pch.hpp>
#include "Gui/ComponentInspectors.hpp"
#include "Gui/ImGuiUtils.hpp"

#include "Components/Components.hpp"
#include "Graphics/TextureManager.hpp"
#include "Graphics/Texture2D.hpp"
#include "Physics/PhysicsTypes.hpp"

#include <imgui.h>
#include <cstdio>
#include <filesystem>
#include <algorithm>

namespace Bolt {

	void DrawNameComponentInspector(Entity entity)
	{
		auto& name = entity.GetComponent<NameComponent>();
		char buffer[256]{};
		std::snprintf(buffer, sizeof(buffer), "%s", name.Name.c_str());
		if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
			name.Name = buffer;
		}
	}

	void DrawTransform2DInspector(Entity entity)
	{
		auto& transform = entity.GetComponent<Transform2DComponent>();
		ImGui::DragFloat2("Position", &transform.Position.x, 0.05f);
		ImGui::DragFloat2("Scale", &transform.Scale.x, 0.05f, 0.001f);
		ImGui::DragFloat("Rotation", &transform.Rotation, 0.01f);
	}

	void DrawRigidbody2DInspector(Entity entity)
	{
		auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

		Vec2 position = rb2D.GetPosition();
		Vec2 velocity = rb2D.GetVelocity();
		float gravityScale = rb2D.GetGravityScale();
		float rotation = rb2D.GetRotation();

		if (ImGui::DragFloat2("Position", &position.x, 0.05f))
			rb2D.SetPosition(position);

		if (ImGui::DragFloat2("Velocity", &velocity.x, 0.05f))
			rb2D.SetVelocity(velocity);

		if (ImGui::DragFloat("Rotation", &rotation, 0.01f))
			rb2D.SetRotation(rotation);

		if (ImGui::SliderFloat("Gravity Scale", &gravityScale, 0.0f, 1.0f))
			rb2D.SetGravityScale(gravityScale);

		ImGuiUtils::DrawEnumCombo<BodyType>("Body Type", rb2D.GetBodyType(), [&rb2D](BodyType newType) {
			rb2D.SetBodyType(newType);
			});
	}

	void DrawSpriteRendererInspector(Entity entity)
	{
		auto& sprite = entity.GetComponent<SpriteRendererComponent>();
		ImGui::ColorEdit4("Color", &sprite.Color.r, ImGuiColorEditFlags_NoInputs);
		ImGui::DragScalar("Sorting Order", ImGuiDataType_S16, &sprite.SortingOrder, 1.0f);
		ImGui::DragScalar("Sorting Layer", ImGuiDataType_U8, &sprite.SortingLayer, 1.0f);

		
		if (ImGui::Button(sprite.TextureHandle.IsValid() ? "Change Texture..." : "Assign Texture...",
			ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			// Button should open the asset browser with a filter for only images and without any directory structure
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				std::string droppedPath(static_cast<const char*>(payload->Data));

				
				std::string ext = std::filesystem::path(droppedPath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
					std::string filename = std::filesystem::path(droppedPath).filename().string();
					sprite.TextureHandle = TextureManager::LoadTexture(droppedPath.c_str());
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (sprite.TextureHandle.IsValid()) {
			if (Texture2D* texture = TextureManager::GetTexture(sprite.TextureHandle)) {
				const float texWidth = texture->GetWidth();
				const float texHeight = texture->GetHeight();

				ImGuiUtils::DrawTexturePreview(texture->GetHandle(), texWidth, texHeight);
				ImGui::Text("%.0f x %.0f", texWidth, texHeight);

				ImGui::PushID("TextureSettings");
				ImGuiUtils::DrawEnumCombo<Filter>("Filter", texture->GetFilter(), [&texture](Filter newFilter) {
					texture->SetFilter(newFilter);
					});
				ImGuiUtils::DrawEnumCombo<Wrap>("Wrap U", texture->GetWrapU(), [&texture](Wrap wrapU) {
					texture->SetWrapU(wrapU);
					});
				ImGuiUtils::DrawEnumCombo<Wrap>("Wrap V", texture->GetWrapV(), [&texture](Wrap wrapV) {
					texture->SetWrapV(wrapV);
					});
				ImGui::PopID();
			}
		}
	}

	void DrawCamera2DInspector(Entity entity)
	{
		auto& camera = entity.GetComponent<Camera2DComponent>();
		float zoom = camera.GetZoom();
		if (ImGui::DragFloat("Zoom", &zoom, 0.01f, 0.01f, 100.0f)) {
			camera.SetZoom(zoom);
		}
		float ortho = camera.GetOrthographicSize();
		if (ImGui::DragFloat("Orthographic Size", &ortho, 0.05f, 0.05f, 1000.0f)) {
			camera.SetOrthographicSize(ortho);
		}

		ImGuiUtils::DrawVec2ReadOnly("Viewport Size", camera.GetViewport()->GetSize());
		ImGuiUtils::DrawVec2ReadOnly("World Viewport", camera.WorldViewPort());
	}

	void DrawParticleSystem2DInspector(Entity entity)
	{
		auto& ps = entity.GetComponent<ParticleSystem2DComponent>();

		if (ImGui::Button(ps.IsPlaying() ? "Pause" : "Play")) {
			if (ps.IsPlaying()) {
				ps.Pause();
			}
			else {
				ps.Play();
			}
		}

		ImGui::InputFloat("LifeTime", &ps.ParticleSettings.LifeTime);
		ImGui::InputFloat("Scale", &ps.ParticleSettings.Scale);
		ImGui::InputFloat("Speed", &ps.ParticleSettings.Speed);

		if (ImGui::RadioButton("Gravity", ps.ParticleSettings.UseGravity))
			ps.ParticleSettings.UseGravity = !ps.ParticleSettings.UseGravity;

		ImGuiUtils::DrawEnabled(ps.ParticleSettings.UseGravity, [&ps]() {
			ImGui::InputFloat2("Gravity Value", &ps.ParticleSettings.Gravity.x);
			});

		if (ImGui::RadioButton("Random Colors", ps.ParticleSettings.UseRandomColors))
			ps.ParticleSettings.UseRandomColors = !ps.ParticleSettings.UseRandomColors;

		if (ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID("Emission");
			ImGui::InputInt("Emit Over Time", (int*)&ps.EmissionSettings.EmitOverTime);

			ImGuiUtils::DrawEnumCombo<ParticleSystem2DComponent::ShapeType>("Shape Type", std::visit([](auto&& shape) -> ParticleSystem2DComponent::ShapeType {
				using T = std::decay_t<decltype(shape)>;
				if constexpr (std::is_same_v<T, ParticleSystem2DComponent::CircleParams>) {
					return ParticleSystem2DComponent::ShapeType::Circle;
				}
				else if constexpr (std::is_same_v<T, ParticleSystem2DComponent::SquareParams>) {
					return ParticleSystem2DComponent::ShapeType::Square;
				}
				}, ps.Shape), [&ps](ParticleSystem2DComponent::ShapeType newType) {
					if (newType == ParticleSystem2DComponent::ShapeType::Circle) {
						ps.Shape = ParticleSystem2DComponent::CircleParams{ 1.f, false };
					}
					else if (newType == ParticleSystem2DComponent::ShapeType::Square) {
						ps.Shape = ParticleSystem2DComponent::SquareParams{ Vec2{ 1.f, 1.f } };
					}
					});

				std::visit([&](auto& s) {
					using T = std::decay_t<decltype(s)>;
					if constexpr (std::is_same_v<T, ParticleSystem2DComponent::CircleParams>) {
						ImGui::InputFloat("Radius", &s.Radius);
						ImGui::Checkbox("On Circle Edge", &s.IsOnCircle);
					}
					else if constexpr (std::is_same_v<T, ParticleSystem2DComponent::SquareParams>) {
						ImGuiUtils::DrawVec2("Half Extents", s.HalfExtends);
					}
					}, ps.Shape);
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Rendering")) {
			ImGui::PushID("Rendering");
			ps.RenderingSettings.Color = ImGuiUtils::DrawColorPick4("Color", ps.RenderingSettings.Color);
			ImGui::DragInt("Max Particles", (int*)&ps.RenderingSettings.MaxParticles, 1.0f, 1.0f, 10000.0f);
			ImGui::InputInt("Sorting Order", (int*)&ps.RenderingSettings.SortingOrder);
			ImGui::InputInt("Sorting Layer", (int*)&ps.RenderingSettings.SortingLayer);

			if (Texture2D* texture = TextureManager::GetTexture(ps.GetTextureHandle())) {
				ImGuiUtils::DrawTexturePreview(texture->GetHandle(), texture->GetWidth(), texture->GetHeight());
			}
			ImGui::PopID();
		}
	}

	void DrawBoxCollider2DInspector(Entity entity)
	{
		(void)entity;
		ImGui::TextDisabled("Box collider properties coming soon");
	}

	void DrawAudioSourceInspector(Entity entity)
	{
		auto& audio = entity.GetComponent<AudioSourceComponent>();

		float volume = audio.GetVolume();
		if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
			audio.SetVolume(volume);
		}

		float pitch = audio.GetPitch();
		if (ImGui::DragFloat("Pitch", &pitch, 0.01f, 0.1f, 3.0f)) {
			audio.SetPitch(pitch);
		}

		bool loop = audio.IsLooping();
		if (ImGui::Checkbox("Loop", &loop)) {
			audio.SetLoop(loop);
		}
	}

}
