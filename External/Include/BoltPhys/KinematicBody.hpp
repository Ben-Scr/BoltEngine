#pragma once
#include "Export.hpp"
#include "Body.hpp"

namespace BoltPhys {
    class BOLT_PHYS_API KinematicBody final : public Body
    {
    public:
        KinematicBody();
    };
}