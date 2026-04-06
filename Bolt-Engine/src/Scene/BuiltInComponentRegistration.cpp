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
			ComponentCategory category = ComponentCategory::Component)
		{
			ComponentInfo info{ displayName, category };
			info.drawInspector = inspector;
			sceneManager.RegisterComponentType<T>(info);
		}
	}

	void RegisterBuiltInComponents(SceneManager& sceneManager) {
		RegisterComponent<Transform2DComponent>(sceneManager, "Transform 2D", DrawTransform2DInspector);
		RegisterComponent<RectTransformComponent>(sceneManager, "Rect Transform");
		RegisterComponent<NameComponent>(sceneManager, "Name", DrawNameComponentInspector);

		RegisterComponent<SpriteRendererComponent>(sceneManager, "Sprite Renderer", DrawSpriteRendererInspector);
		RegisterComponent<ImageComponent>(sceneManager, "Image");
		RegisterComponent<Camera2DComponent>(sceneManager, "Camera 2D", DrawCamera2DInspector);
		RegisterComponent<ParticleSystem2DComponent>(sceneManager, "Particle System 2D", DrawParticleSystem2DInspector);

		RegisterComponent<BoxCollider2DComponent>(sceneManager, "Box Collider 2D", DrawBoxCollider2DInspector);
		RegisterComponent<Rigidbody2DComponent>(sceneManager, "Rigidbody 2D", DrawRigidbody2DInspector);

		RegisterComponent<AudioSourceComponent>(sceneManager, "Audio Source", DrawAudioSourceInspector);

		//RegisterComponent<ScriptComponent>(sceneManager, "Scripts", DrawScriptComponentInspector);

		RegisterComponent<IdTag>(sceneManager, "Id", nullptr, ComponentCategory::Tag);
		RegisterComponent<StaticTag>(sceneManager, "Static", nullptr, ComponentCategory::Tag);
		RegisterComponent<DisabledTag>(sceneManager, "Disabled", nullptr, ComponentCategory::Tag);
		RegisterComponent<DeadlyTag>(sceneManager, "Deadly", nullptr, ComponentCategory::Tag);
	}
}
