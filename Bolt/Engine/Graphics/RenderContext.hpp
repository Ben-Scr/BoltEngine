#pragma once

namespace Bolt {

    enum class ViewID : uint16_t { Main = 0, Gizmos = 1, UI = 2, Count };

    //class RenderContext {
    //public:
    //    RenderContext() = default;
    //    ~RenderContext() = default;

    //    bool Init(const bgfx::PlatformData& pd, int width, int height, uint32_t resetFlags = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4);


    //    void Reset(int width, int height);
    //    void Shutdown();

    //    uint16_t GetViewId(ViewID viewId) const noexcept { return static_cast<uint16_t>(viewId); }
    //    void SetViewCamera(ViewID viewId, const float view[16], const float proj[16]) const;
    //    void SetViewClear(ViewID viewId, uint16_t clearFlags, uint32_t rgba, float depth = 1.0f, uint8_t stencil = 0) const;
    //    void TouchView(ViewID viewId) const noexcept { bgfx::touch(GetViewId(viewId)); }
    //    void SetViewRect(ViewID viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height) const;

    //    int Width() const noexcept { return m_width; }
    //    int Height() const noexcept { return m_height; }
    //    bool IsInitialized() const noexcept { return m_initialized; }

    //private:
    //    uint32_t m_resetFlags = BGFX_RESET_NONE;
    //    bool     m_initialized = false;
    //    int      m_width = 0;
    //    int      m_height = 0;
    //};

}