#include "pch.hpp"
#include "Scene/BuiltInComponentRegistration.hpp"

#include "Scene/SceneManager.hpp"
#include "Components/Components.hpp"

namespace Bolt {
	namespace {
		template<typename T>
		void RegisterComponent(SceneManager& sceneManager, const std::string& displayName, ComponentCategory category = ComponentCategory::Component) {
			sceneManager.RegisterComponentType<T>(ComponentInfo{ displayName, category });
		}
	}

	void RegisterBuiltInComponents(SceneManager& sceneManager) {
		RegisterComponent<Transform2DComponent>(sceneManager, "Transform 2D");
		RegisterComponent<RectTransformComponent>(sceneManager, "Rect Transform");
		RegisterComponent<NameComponent>(sceneManager, "Name");

		RegisterComponent<SpriteRendererComponent>(sceneManager, "Sprite Renderer");
		RegisterComponent<ImageComponent>(sceneManager, "Image");
		RegisterComponent<Camera2DComponent>(sceneManager, "Camera 2D");
		RegisterComponent<ParticleSystem2DComponent>(sceneManager, "Particle System 2D");

		RegisterComponent<BoxCollider2DComponent>(sceneManager, "Box Collider 2D");
		RegisterComponent<Rigidbody2DComponent>(sceneManager, "Rigidbody 2D");

		RegisterComponent<AudioSourceComponent>(sceneManager, "Audio Source");

		RegisterComponent<IdTag>(sceneManager, "Id", ComponentCategory::Tag);
		RegisterComponent<StaticTag>(sceneManager, "Static", ComponentCategory::Tag);
		RegisterComponent<DisabledTag>(sceneManager, "Disabled", ComponentCategory::Tag);
		RegisterComponent<DeadlyTag>(sceneManager, "Deadly", ComponentCategory::Tag);
	}
}