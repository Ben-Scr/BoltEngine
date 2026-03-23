#pragma once
#include "Export.hpp"
#include "Body.hpp"

namespace BoltPhys {
    class BOLT_PHYS_API StaticBody final : public Body
    {
    public:
        StaticBody() : Body(BodyType::Static)
        {
            SetBoundaryCheckEnabled(false);
            SetGravityEnabled(false);
        }
    };

    class BOLT_PHYS_API DynamicBody final : public Body
    {
    public:
        DynamicBody() : Body(BodyType::Dynamic) {}
    };

    class BOLT_PHYS_API KinematicBody final : public Body
    {
    public:
        KinematicBody() : Body(BodyType::Kinematic)
        {
            SetGravityEnabled(false);
        }
    };
}