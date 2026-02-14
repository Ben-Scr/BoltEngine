#pragma once

#include "Debugging/Exceptions.hpp"
#include "Scene/EntityHandle.hpp"
#include "Scene/Entity.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/ComponentUtils.hpp"
#include "Core/Core.hpp"

namespace Bolt {
	class BOLT_API EntityHelper {
	public:
        // Info: Creates an entity with the entered Components
        template<typename... Components>
        static Entity CreateWith() {
            Scene& activeScene = *SceneManager::GetActiveScene();

            BOLT_ASSERT(activeScene.IsLoaded(), 
                BoltErrorCode::Undefined,
                "There is no active Scene Loaded");

            Entity entity(activeScene.CreateEntityHandle(), activeScene.GetRegistry());

            (entity.AddComponent<Components>(), ...);

            return entity;
        }

        template<typename... Components>
        static EntityHandle CreateHandleWith() {
            Scene& activeScene = *SceneManager::GetActiveScene();

            BOLT_ASSERT(activeScene.IsLoaded(),
                BoltErrorCode::Undefined,
                "There is no active Scene Loaded");

            EntityHandle entity = activeScene.CreateEntityHandle();

            (activeScene.AddComponent<Components>(entity), ...);
            return entity;
        }

        static void SetEnabled(Entity entity, bool enabled);
        static bool IsEnabled(Entity entity);


        // Info: Basically calls CreateWith<Transform2D, Canera2D>();
        static Entity CreateCamera2DEntity();
        // Info: Basically calls CreateWith<Transform2D, SpriteRenderer>();
        static Entity CreateSpriteEntity();
        // Info: Basically calls CreateWith<RectTransform, Image>();
        static Entity CreateImageEntity();
	};

}