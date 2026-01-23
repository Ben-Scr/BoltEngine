#pragma once
#include "Vec2.hpp"

namespace Bolt {
    struct Viewport {
    public:
		Viewport() = default;
        Viewport(int width, int height) : Width{ width }, Height{ height } {}

        void SetSize(int width, int height) { Width = width, Height = height; }
        Vec2Int GetSize() const noexcept { return Vec2Int{ Width, Height }; }
        float GetAspect() const noexcept { return static_cast<float>(Width) / static_cast<float>(Height); }
        Vec2  GetHalfSize() const noexcept { return Vec2(0.5f * Width, 0.5f * Height); }
        Vec2  GetCenter() const noexcept { return Vec2(0.5f * Width, 0.5f * Height); }

        int   Width = 1;
        int   Height = 1;
    };
}