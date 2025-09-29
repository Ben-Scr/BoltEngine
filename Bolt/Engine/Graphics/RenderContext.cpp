#include "../pch.hpp"
#include "RenderContext.hpp"


namespace Bolt {

    //bool RenderContext::Init(const bgfx::PlatformData& pd, int width, int height, uint32_t resetFlags) {
    //    if (m_initialized) return true;
    //    if (width <= 0 || height <= 0) {
    //        Logger::Error("[RenderContext] invalid resolution\n");
    //        return false;
    //    }

    //    m_width = width;
    //    m_height = height;
    //    m_resetFlags = resetFlags;

    //    bgfx::setPlatformData(pd);

    //    bgfx::Init init{};
    //    init.type = bgfx::RendererType::Count;
    //    init.platformData = pd;
    //    init.resolution.width = static_cast<uint32_t>(m_width);
    //    init.resolution.height = static_cast<uint32_t>(m_height);
    //    init.resolution.reset = m_resetFlags;

    //    if (!bgfx::init(init)) {
    //        Logger::Error("[RenderContext] bgfx::init failed\n");
    //        return false;
    //    }


    //    for (uint16_t i = 0; i < static_cast<uint16_t>(ViewID::Count); ++i) {
    //        bgfx::setViewRect(i, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    //    }

    //    m_initialized = true;
    //    return true;
    //}

    //void RenderContext::Reset(int width, int height) {
    //    if (!m_initialized) return;
    //    if (width <= 0 || height <= 0) return;

    //    m_width = width;
    //    m_height = height;
    //    bgfx::reset(static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), m_resetFlags);


    //    for (uint16_t i = 0; i < static_cast<uint16_t>(ViewID::Count); ++i) {
    //        bgfx::setViewRect(i, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    //    }
    //}

    //void RenderContext::Shutdown() {
    //    if (!m_initialized) return;
    //    bgfx::shutdown();
    //    m_initialized = false;
    //    m_width = m_height = 0;
    //}

    //void RenderContext::SetViewCamera(ViewID viewId, const float view[16], const float proj[16]) const {
    //    bgfx::setViewTransform(GetViewId(viewId), view, proj);
    //    bgfx::setViewRect(GetViewId(viewId), 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    //}

    //void RenderContext::SetViewClear(ViewID viewId, uint16_t clearFlags, uint32_t rgba, float depth, uint8_t stencil) const {
    //    bgfx::setViewClear(GetViewId(viewId), clearFlags, rgba, depth, stencil);
    //    bgfx::setViewRect(GetViewId(viewId), 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    //}

    //void RenderContext::SetViewRect(ViewID viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height) const {
    //    bgfx::setViewRect(GetViewId(viewId), x, y, width, height);
    //}

}
