#pragma once
#include "Export.hpp"
#include "WorldSettings.hpp"
#include "Body2D.hpp"
#include "Collider2D.hpp"
#include "Contact.hpp"

#include <cstddef>
#include <vector>

namespace BoltPhys {
    class BOLT_PHYS_API PhysicsWorld
    {
    public:
        PhysicsWorld();
        explicit PhysicsWorld(const WorldSettings& settings);

        void SetSettings(const WorldSettings& settings) noexcept;
        const WorldSettings& GetSettings() const noexcept;

        bool RegisterBody(Body2D& body);
        bool UnregisterBody(Body2D& body);

        bool RegisterCollider(Collider2D& collider);
        bool UnregisterCollider(Collider2D& collider);

        bool AttachCollider(Body2D& body, Collider2D& collider);
        void DetachCollider(Body2D& body);

        void Step(float dt);

        std::size_t GetBodyCount() const noexcept;
        std::size_t GetColliderCount() const noexcept;
        const std::vector<Collider2D*>& GetColliders() const noexcept;
        const std::vector<Contact>& GetContacts() const noexcept;

    private:
        void IntegrateBodies(float dt);
        void ApplyWorldBounds(Body2D& body) const noexcept;
        void DetectCollisions();
        void ResolveContacts();

        Contact BuildContact(Body2D& bodyA, Body2D& bodyB) const;

        WorldSettings m_settings;
        std::vector<Body2D*> m_bodies;
        std::vector<Collider2D*> m_colliders;
        std::vector<Contact> m_contacts;
    };
}