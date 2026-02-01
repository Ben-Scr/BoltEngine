#include "pch.hpp"
#include "EntityHelper.hpp"

#include "Graphics/Camera2D.hpp"
#include "Components/SpriteRenderer.hpp"
#include "Components/RectTransform.hpp"
#include "Components/Image.hpp"
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
		return CreateWith<Transform2D, Camera2D>();
	}

	Entity EntityHelper::CreateSpriteEntity() {
		return CreateWith<Transform2D, SpriteRenderer >();
	}

	Entity EntityHelper::CreateImageEntity() {
		return CreateWith<RectTransform, Image>();
	}
}