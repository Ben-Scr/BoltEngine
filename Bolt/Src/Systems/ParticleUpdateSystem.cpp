#pragma once
#include "pch.hpp"
#include "ParticleUpdateSystem.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"

#include "Components/ParticleSystem2DComponent.hpp"
#include "Components/Tags.hpp"

#include <entt/entt.hpp>

namespace Bolt {
	void ParticleUpdateSystem::Update(Scene& scene) {
		for (const auto& [ent, particleSystem] : scene.GetRegistry().view<ParticleSystem2DComponent>(entt::exclude<DisabledTag>).each())
			particleSystem.Update();
	}
}