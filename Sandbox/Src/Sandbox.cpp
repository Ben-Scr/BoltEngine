#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include "GameSystem.hpp"
#include <Scene/EntityHelper.hpp>

#include <iostream>

#include <BoltPhys/Body2D.hpp>
#include <BoltPhys/BodyType.hpp>
#include <BoltPhys/Collider2D.hpp>
#include <BoltPhys/PhysicsWorld.hpp>
#include <BoltPhys/BoxCollider2D.hpp>

#include <Graphics/Gizmos.hpp>

#include <Core/Time.hpp>
#include <Core/Input.hpp>

using namespace Bolt;
using namespace BoltPhys;


class Sandbox : public Bolt::Application {
public:
	PhysicsWorld world;
	Entity player{ Entity::Null };

	Entity CreatePhysicsEntity(Transform2DComponent tr2D, BoltPhys::BodyType type) {
		Entity entity = EntityHelper::CreateWith<SpriteRendererComponent, Transform2DComponent>();
		entity.GetComponent<Transform2DComponent>() = tr2D;
		Body2D& body2D = entity.AddComponent<Body2D>();
		body2D.SetBodyType(type);
		body2D.SetPosition(tr2D.Position);

		BoxCollider2D& boxCollider = entity.AddComponent<BoltPhys::BoxCollider2D>();
		boxCollider.SetHalfExtents(tr2D.Scale / 2.0f);

		world.RegisterBody(body2D);
		world.RegisterCollider(boxCollider);
		world.AttachCollider(body2D, boxCollider);
		return entity;
	}

	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("Game");
		//def.AddSystem<GameSystem>();
		//def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~Sandbox() override = default;

	void Start() override {
		EntityHelper::CreateCamera2DEntity();

		player = CreatePhysicsEntity(Transform2DComponent({ 0.f, 5.f }, { 1.f, 1.f }), BoltPhys::BodyType::Dynamic);
		CreatePhysicsEntity(Transform2DComponent({ 0.f, -1.f }, { 10.f, 1.f }), BoltPhys::BodyType::Static);
	}
	void Update() override {
		Scene& activeScene = *SceneManager::Get().GetActiveScene();
		for (const auto& [ent, tr2D, box2D] : activeScene.GetRegistry().view<Transform2DComponent, BoxCollider2D>().each()) {
			Bolt::Gizmo::DrawSquare(box2D.GetBody()->GetPosition(), box2D.GetHalfExtents() * 2.0f, 0);
			tr2D.Position = box2D.GetBody()->GetPosition();
		}

		if (Input::GetKeyDown(KeyCode::E)) {
			CreatePhysicsEntity(Transform2DComponent({ 0.f, 5.f }, { 1.f, 1.f }), BoltPhys::BodyType::Dynamic);
		}

		Vec2 axis = Input::GetAxis();
		float speed = 5.0f;

		Body2D& body2D = player.GetComponent<Body2D>();
		body2D.SetVelocity({ axis.x * 5.0f, body2D.GetVelocity().y });

		if (Input::GetKeyDown(KeyCode::Space)) {
			Body2D& body2D = player.GetComponent<Body2D>();
			body2D.SetVelocity({ body2D.GetVelocity().x, 5.0f });
		}
	}

	void FixedUpdate() override {
		world.Step(Bolt::Time::GetFixedDeltaTime());
	}

	void OnPaused() override {

	}
	void OnQuit() override {
		Logger::Message("Quit");
	}
};



Bolt::Application* Bolt::CreateApplication() {
	return new Sandbox();
}