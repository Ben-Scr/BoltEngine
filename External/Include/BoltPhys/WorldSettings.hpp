#pragma once
#include "Export.hpp"
#include "Vec2.hpp"

namespace BoltPhys {
    struct BOLT_PHYS_API WorldSettings
    {
        Vec2 gravity{ 0.0f, -9.81f };
        Vec2 worldMin{ -1000.0f, -1000.0f };
        Vec2 worldMax{ 1000.0f,  1000.0f };
        bool enableWorldBounds = false;
    };
}