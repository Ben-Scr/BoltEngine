#include "../pch.hpp"
#include "RenderResources.hpp"

namespace Bolt {
 /*   bool RenderResources::Init() {
        m_PosTexLayout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();


        m_PosColorLayout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, false)
            .end();



        m_SamplerUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(m_SamplerUniform)) {
            std::cerr << "[RenderResources] Failed to create sampler uniform\n";
            return false;
        }


        uint8_t white[] = { 0xFF, 0xFF, 0xFF, 0xFF };
        const bgfx::Memory* mem = bgfx::makeRef(white, sizeof(white));
        m_DefaultTexture = bgfx::createTexture2D(
            1, 1, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE, mem
        );

        if (!bgfx::isValid(m_DefaultTexture)) {
            std::cerr << "[RenderResources] Failed to create default white texture\n";
            return false;
        }

        return true;
    }

    void RenderResources::Shutdown() {
        if (bgfx::isValid(m_SamplerUniform)) {
            bgfx::destroy(m_SamplerUniform);
            m_SamplerUniform = BGFX_INVALID_HANDLE;
        }

        if (bgfx::isValid(m_DefaultTexture)) {
            bgfx::destroy(m_DefaultTexture);
            m_DefaultTexture = BGFX_INVALID_HANDLE;
        }
    }*/
}