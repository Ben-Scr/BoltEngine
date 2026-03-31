#pragma once
#include "Physics/CollisionDispatcher.hpp"
#include "Scene/EntityHandle.hpp"
#include "Physics/PhysicsTypes.hpp"
#include <box2d/box2d.h>

namespace Bolt {
    class Scene;
}

namespace Bolt {

    class Box2DWorld {
        friend class Physics2D;

    public:
        Box2DWorld();
        ~Box2DWorld();

        Box2DWorld(const Box2DWorld&) = delete;
        Box2DWorld& operator=(const Box2DWorld&) = delete;

        Box2DWorld(Box2DWorld&&) noexcept = default;
        Box2DWorld& operator=(Box2DWorld&&) noexcept = default;

        void Step(float dt);


        b2BodyId CreateBody(EntityHandle nativeEntity, Scene& scene, BodyType bodyType);
        b2ShapeId CreateShape(EntityHandle nativeEntity, Scene& scene, b2BodyId bodyId, ShapeType shapeType);

        CollisionDispatcher& GetDispatcher();
        b2WorldId GetWorldID() { return m_WorldId; }
        void Destroy();
    private:
        b2WorldId m_WorldId;
        CollisionDispatcher m_Dispatcher{};
    };
}