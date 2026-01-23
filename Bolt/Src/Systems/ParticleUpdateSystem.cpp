#pragma once
#include "../pch.hpp"
#include "ParticleUpdateSystem.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"

#include "../Components/ParticleSystem2D.hpp"
#include "../Components/Tags.hpp"

#include <entt/entt.hpp>

namespace Bolt {
	void ParticleUpdateSystem::Update() {
		for (const auto& [ent, particleSystem] : SceneManager::GetActiveScene()->GetRegistry().view<ParticleSystem2D>(entt::exclude<DisabledTag>).each())
			particleSystem.Update();
	}
}