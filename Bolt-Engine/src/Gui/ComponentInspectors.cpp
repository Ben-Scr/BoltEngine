#include <pch.hpp>
#include "Gui/ComponentInspectors.hpp"
#include "Gui/ImGuiUtils.hpp"

#include "Components/Components.hpp"
#include "Graphics/TextureManager.hpp"
#include "Graphics/Texture2D.hpp"
#include "Physics/PhysicsTypes.hpp"

#include <imgui.h>
#include <cstdio>

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
		ImGui::DragFloat2("Position##Transform2D", &transform.Position.x, 0.05f);
		ImGui::DragFloat2("Scale##Transform2D", &transform.Scale.x, 0.05f, 0.001f);
		ImGui::DragFloat("Rotation##Transform2D", &transform.Rotation, 0.01f);
	}

	void DrawRigidbody2DInspector(Entity entity)
	{
		auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

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

		if (ImGui::SliderFloat("Gravity Scale##RigidBody2D", &gravityScale, 0.0f, 1.0f))
			rb2D.SetGravityScale(gravityScale);

		ImGuiUtils::DrawEnumCombo<BodyType>("Body Type##RigidBody2D", rb2D.GetBodyType(), [&rb2D](BodyType newType) {
			rb2D.SetBodyType(newType);
		});
	}

	void DrawSpriteRendererInspector(Entity entity)
	{
		auto& sprite = entity.GetComponent<SpriteRendererComponent>();
		ImGui::ColorEdit4("Color", &sprite.Color.r, ImGuiColorEditFlags_NoInputs);
		ImGui::DragScalar("Sorting Order", ImGuiDataType_S16, &sprite.SortingOrder, 1.0f);
		ImGui::DragScalar("Sorting Layer", ImGuiDataType_U8, &sprite.SortingLayer, 1.0f);

		if (sprite.TextureHandle.IsValid()) {
			if (Texture2D* texture = TextureManager::GetTexture(sprite.TextureHandle)) {
				const float texWidth = texture->GetWidth();
				const float texHeight = texture->GetHeight();

				ImGuiUtils::DrawTexturePreview(texture->GetHandle(), texWidth, texHeight);
				ImGui::Text("%.0f x %.0f", texWidth, texHeight);

				ImGuiUtils::DrawEnumCombo<Filter>("Filter##Texture2D", texture->GetFilter(), [&texture](Filter newFilter) {
					texture->SetFilter(newFilter);
				});
				ImGuiUtils::DrawEnumCombo<Wrap>("Wrap U##Texture2D", texture->GetWrapU(), [&texture](Wrap wrapU) {
					texture->SetWrapU(wrapU);
				});
				ImGuiUtils::DrawEnumCombo<Wrap>("Wrap V##Texture2D", texture->GetWrapV(), [&texture](Wrap wrapV) {
					texture->SetWrapV(wrapV);
				});
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
		ImGuiUtils::DrawVec2ReadOnly("World Viewport Size", camera.WorldViewPort());
	}

	void DrawParticleSystem2DInspector(Entity entity)
	{
		(void)entity;
		// TODO: Particle system inspector fields
	}

	void DrawBoxCollider2DInspector(Entity entity)
	{
		(void)entity;
		// TODO: Box collider inspector fields
	}

	void DrawAudioSourceInspector(Entity entity)
	{
		auto& audio = entity.GetComponent<AudioSourceComponent>();

		float volume = audio.GetVolume();
		if (ImGui::SliderFloat("Volume##AudioSource", &volume, 0.0f, 1.0f)) {
			audio.SetVolume(volume);
		}

		float pitch = audio.GetPitch();
		if (ImGui::DragFloat("Pitch##AudioSource", &pitch, 0.01f, 0.1f, 3.0f)) {
			audio.SetPitch(pitch);
		}

		bool loop = audio.IsLooping();
		if (ImGui::Checkbox("Loop##AudioSource", &loop)) {
			audio.SetLoop(loop);
		}
	}

} // namespace Bolt
