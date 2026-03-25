#include "pch.hpp"
#include "EntityHelper.hpp"

#include "Components/Graphics/Camera2DComponent.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/General/RectTransformComponent.hpp"
#include "Components/Graphics/ImageComponent.hpp"
#include "Components/Tags.hpp"

namespace Bolt {
	void EntityHelper::SetEnabled(Entity entity, bool enabled) {
		if (!enabled && !entity.HasComponent<DisabledTag>())
			entity.AddComponent<DisabledTag>();
		else if (enabled && entity.HasComponent<DisabledTag>())
			entity.RemoveComponent<DisabledTag>();
	}
	bool EntityHelper::IsEnabled(Entity entity) {
		return !entity.HasComponent<DisabledTag>();
	}


	Entity EntityHelper::CreateCamera2DEntity() {
		return CreateWith<Transform2DComponent, Camera2DComponent>();
	}

	Entity EntityHelper::CreateSpriteEntity() {
		return CreateWith<Transform2DComponent, SpriteRendererComponent >();
	}

	Entity EntityHelper::CreateImageEntity() {
		return CreateWith<RectTransform, ImageComponent>();
	}
}