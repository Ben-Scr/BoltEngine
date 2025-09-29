#pragma once
#include "Vec2.hpp"

namespace Bolt {
    class Viewport {
    public:
		Viewport() = default;
        Viewport(int width, int height) { SetSize(width, height); }

        void SetSize(int width, int height) {
            m_Width = std::max(1, width);
            m_Height = std::max(1, height);
            m_Aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        }

        int  GetWidth()  const noexcept { return m_Width; }
        int  GetHeight() const noexcept { return m_Height; }


        Vec2Int GetSize() const noexcept { return Vec2Int{ m_Width, m_Height }; }

        float GetAspect() const noexcept { return m_Aspect; }
        Vec2  GetHalfSize() const noexcept { return Vec2(0.5f * m_Width, 0.5f * m_Height); }
        Vec2  GetCenter() const noexcept { return Vec2(0.5f * m_Width, 0.5f * m_Height); }

    private:
        int   m_Width = 1;
        int   m_Height = 1;
		float m_Aspect = 1.0f;
    };
}