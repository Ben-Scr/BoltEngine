#include "pch.hpp"
#include "Scene/BuiltInComponentRegistration.hpp"

#include "Scene/SceneManager.hpp"
#include "Components/Components.hpp"
#include "Gui/ComponentInspectors.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/ScriptComponentInspector.hpp"

namespace Bolt {
	namespace {
		template<typename T>
		void RegisterComponent(SceneManager& sceneManager, const std::string& displayName,
			void (*inspector)(Entity) = nullptr,
			ComponentCategory category = ComponentCategory::Component,
			const std::string& subcategory = "")
		{
			ComponentInfo info{ displayName, subcategory, category };
			info.drawInspector = inspector;
			sceneManager.RegisterComponentType<T>(info);
		}
	}

	void RegisterBuiltInComponents(SceneManager& sceneManager) {
		// General
		RegisterComponent<Transform2DComponent>(sceneManager, "Transform 2D", DrawTransform2DInspector, ComponentCategory::Component, "General");
		RegisterComponent<RectTransformComponent>(sceneManager, "Rect Transform", nullptr, ComponentCategory::Component, "General");
		RegisterComponent<NameComponent>(sceneManager, "Name", DrawNameComponentInspector, ComponentCategory::Component, "General");

		// Rendering
		RegisterComponent<SpriteRendererComponent>(sceneManager, "Sprite Renderer", DrawSpriteRendererInspector, ComponentCategory::Component, "Rendering");
		RegisterComponent<ImageComponent>(sceneManager, "Image", nullptr, ComponentCategory::Component, "Rendering");
		RegisterComponent<Camera2DComponent>(sceneManager, "Camera 2D", DrawCamera2DInspector, ComponentCategory::Component, "Rendering");
		RegisterComponent<ParticleSystem2DComponent>(sceneManager, "Particle System 2D", DrawParticleSystem2DInspector, ComponentCategory::Component, "Rendering");

		// Physics
		RegisterComponent<BoxCollider2DComponent>(sceneManager, "Box Collider 2D", DrawBoxCollider2DInspector, ComponentCategory::Component, "Physics");
		RegisterComponent<Rigidbody2DComponent>(sceneManager, "Rigidbody 2D", DrawRigidbody2DInspector, ComponentCategory::Component, "Physics");

		// Bolt-Physics components (lightweight AABB physics)
		RegisterComponent<BoltBody2DComponent>(sceneManager, "Bolt Body 2D", DrawBoltBody2DInspector, ComponentCategory::Component, "Physics");
		RegisterComponent<BoltBoxCollider2DComponent>(sceneManager, "Bolt Box Collider 2D", DrawBoltBoxCollider2DInspector, ComponentCategory::Component, "Physics");
		RegisterComponent<BoltCircleCollider2DComponent>(sceneManager, "Bolt Circle Collider 2D", DrawBoltCircleCollider2DInspector, ComponentCategory::Component, "Physics");

		// Audio
		RegisterComponent<AudioSourceComponent>(sceneManager, "Audio Source", DrawAudioSourceInspector, ComponentCategory::Component, "Audio");

		// Scripting
		RegisterComponent<ScriptComponent>(sceneManager, "Scripts", DrawScriptComponentInspector, ComponentCategory::Component, "Scripting");

		// Tags (not user-addable)
		RegisterComponent<IdTag>(sceneManager, "Id", nullptr, ComponentCategory::Tag);
		RegisterComponent<StaticTag>(sceneManager, "Static", nullptr, ComponentCategory::Tag);
		RegisterComponent<DisabledTag>(sceneManager, "Disabled", nullptr, ComponentCategory::Tag);
		RegisterComponent<DeadlyTag>(sceneManager, "Deadly", nullptr, ComponentCategory::Tag);
	}
}
