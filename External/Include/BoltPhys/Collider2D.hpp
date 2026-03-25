#pragma once
#include "Export.hpp"
#include "ColliderType.hpp"
#include "AABB.hpp"

namespace BoltPhys {
    class Body2D;

    class BOLT_PHYS_API Collider2D
    {
    public:
        Collider2D() = default;
        ~Collider2D() = default;

        virtual void Destroy();

        ColliderType GetType() const noexcept;

        Body2D* GetBody() noexcept;
        const Body2D* GetBody() const noexcept;

        void SetBody(Body2D* body) noexcept;

        virtual AABB ComputeAABB() const = 0;

    protected:
        explicit Collider2D(ColliderType type) noexcept;

    private:
        ColliderType m_type;
        Body2D* m_body = nullptr;
    };
}
