#include "../pch.hpp"
#include "GameSystem.hpp"

namespace Bolt {
    void GameSystem::Awake(Scene& scene) {

    }

    void GameSystem::Start(Scene& scene) {
        Entity camEntity = scene.CreateEntity();
        Transform& cameraTransform = camEntity.AddComponent<Transform>();
        cameraTransform.Position = Vec3(0.0f, 0.0f, -10.0f);

        Camera& camera = camEntity.AddComponent<Camera>();
        camera.SetPerspective(60.0f, 0.01f, 100.0f);
        camera.LookAt(Vec3(0.0f), Vec3(0.0f, 0.0f, 0.0f));

        Entity cubeEntity = scene.CreateEntity();
        Transform& cubeTransform = cubeEntity.AddComponent<Transform>();
        cubeTransform.Position = Vec3(0.0f, 0.0f, 10.0f);
        cubeTransform.Scale = Vec3(1.0f, 1.0f, 1.0f);

        MeshRenderer& meshRenderer = cubeEntity.AddComponent<MeshRenderer>();
        meshRenderer.Mesh = Mesh::Cube();
        meshRenderer.AlbedoColor = Color::White();
    }

    void GameSystem::Update(Scene& scene) {

    }
}