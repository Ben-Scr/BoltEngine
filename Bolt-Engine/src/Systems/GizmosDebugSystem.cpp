#include "pch.hpp"
#include "GizmosDebugSystem.hpp"
#include "Graphics/Gizmos.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"


#include "Components/Physics/BoxCollider2DComponent.hpp"

namespace Bolt {
	void GizmosDebugSystem::OnUpdate(Application& app, float dt) {
		//TODO(Ben-Scr): Bad setup, has to be shifted into the Bolt-Editor code.
		return;

		(void)app;
		(void)dt;

		SceneManager::Get().ForeachLoadedScene([](const Scene& scene) {
			Gizmo::SetColor(Color::Green());

			for (auto [ent, boxCollider] : scene.GetRegistry().view<BoxCollider2DComponent>().each()) {
				Gizmo::DrawSquare(boxCollider.GetBodyPosition(), boxCollider.GetScale(), boxCollider.GetRotationDegrees());
			}

			Gizmo::SetColor(Color::Gray());

			for (auto [ent, tr] : scene.GetRegistry().view<Transform2DComponent>().each()) {
				Gizmo::DrawSquare(tr.Position, tr.Scale, tr.GetRotationDegrees());
			}
		});
	}
}
