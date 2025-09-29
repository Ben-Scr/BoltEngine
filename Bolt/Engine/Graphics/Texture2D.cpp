#include "../pch.hpp"
#include "Texture2D.hpp"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <algorithm>

namespace Bolt {

    // --- Wrap/Filter Mapping ---

    static GLint toGLWrap(Wrap w) {
        switch (w) {
        case Wrap::Repeat: return GL_REPEAT;
        case Wrap::Clamp:  return GL_CLAMP_TO_EDGE;
        case Wrap::Mirror: return GL_MIRRORED_REPEAT;
        case Wrap::Border: return GL_CLAMP_TO_BORDER;
        default:           return GL_CLAMP_TO_EDGE;
        }
    }

    static void chooseInternalAndFormat(int channels, bool srgb, GLint& internalFmt, GLenum& dataFmt) {
        // Unterstützt 1/2/3/4 Kanäle
        switch (channels) {
        case 1: internalFmt = GL_R8;  dataFmt = GL_RED;  break;
        case 2: internalFmt = GL_RG8; dataFmt = GL_RG;   break;     // GL 3.0+
        case 3:
            internalFmt = srgb ? GL_SRGB8 : GL_RGB8;
            dataFmt = GL_RGB;
            break;
        case 4:
        default:
            internalFmt = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            dataFmt = GL_RGBA;
            break;
        }
    }

    // --- Texture2D ---

    Texture2D::Texture2D(const char* path,
        Filter filter, Wrap wrapU, Wrap wrapV,
        bool generateMipmaps, bool srgb, bool flipVertical)
        : m_Filter(filter), m_WrapU(wrapU), m_WrapV(wrapV), m_HasMips(generateMipmaps)
    {
        Load(path, generateMipmaps, srgb, flipVertical);
        if (IsValid()) applySamplerParams();
    }

    Texture2D::~Texture2D() { destroy(); }

    Texture2D::Texture2D(Texture2D&& o) noexcept {
        m_Tex = o.m_Tex; o.m_Tex = 0;
        m_Width = o.m_Width; m_Height = o.m_Height; m_Channels = o.m_Channels;
        m_Filter = o.m_Filter; m_WrapU = o.m_WrapU; m_WrapV = o.m_WrapV; m_HasMips = o.m_HasMips;
    }

    Texture2D& Texture2D::operator=(Texture2D&& o) noexcept {
        if (this != &o) {
            destroy();
            m_Tex = o.m_Tex; o.m_Tex = 0;
            m_Width = o.m_Width; m_Height = o.m_Height; m_Channels = o.m_Channels;
            m_Filter = o.m_Filter; m_WrapU = o.m_WrapU; m_WrapV = o.m_WrapV; m_HasMips = o.m_HasMips;
        }
        return *this;
    }

    void Texture2D::destroy() {
        if (m_Tex) {
            glDeleteTextures(1, &m_Tex);
            m_Tex = 0;
        }
        m_Width = m_Height = m_Channels = 0;
    }

    bool Texture2D::Load(const char* path, bool generateMipmaps, bool srgb, bool flipVertical) {
        // Vorherige Textur ggf. entsorgen
        destroy();

        stbi_set_flip_vertically_on_load(flipVertical);

        int w = 0, h = 0, n = 0;
        unsigned char* pixels = stbi_load(path, &w, &h, &n, 0);
        if (!pixels) {
            Logger::Error(std::string("[Texture2D] Failed to load: ") + path);
            return false;
        }

        GLint internalFmt = GL_RGBA8;
        GLenum dataFmt = GL_RGBA;
        chooseInternalAndFormat(n, srgb, internalFmt, dataFmt);

        // Manche Formate brauchen UNPACK_ALIGNMENT 1 (nicht Vielfaches von 4)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glGenTextures(1, &m_Tex);
        glBindTexture(GL_TEXTURE_2D, m_Tex);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0, dataFmt, GL_UNSIGNED_BYTE, pixels);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Standard-Sampler (wird ggf. durch SetSampler überschrieben)
        m_HasMips = generateMipmaps;
        applySamplerParams();

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(pixels);

        m_Width = w; m_Height = h; m_Channels = n;
        return true;
    }

    void Texture2D::SetSampler(Filter filter, Wrap u, Wrap v) {
        m_Filter = filter;
        m_WrapU = u;
        m_WrapV = v;
        if (IsValid()) applySamplerParams();
    }

    void Texture2D::applySamplerParams() const {
        if (!m_Tex) return;
        glBindTexture(GL_TEXTURE_2D, m_Tex);

        // Wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(m_WrapU));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(m_WrapV));
        if (toGLWrap(m_WrapU) == GL_CLAMP_TO_BORDER || toGLWrap(m_WrapV) == GL_CLAMP_TO_BORDER) {
            const float border[4] = { 0,0,0,0 }; // schwarz transparent
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        }

        // Filter
        GLint mag = (m_Filter == Filter::Point) ? GL_NEAREST : GL_LINEAR;
        GLint minNoMip = (m_Filter == Filter::Point) ? GL_NEAREST : GL_LINEAR;
        GLint minWithMip = GL_NEAREST_MIPMAP_NEAREST;

        switch (m_Filter) {
        case Filter::Point:
            minWithMip = GL_NEAREST_MIPMAP_NEAREST; break;
        case Filter::Bilinear:
            minWithMip = GL_LINEAR_MIPMAP_NEAREST; break;
        case Filter::Trilinear:
        case Filter::Anisotropic:
            minWithMip = GL_LINEAR_MIPMAP_LINEAR;  break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_HasMips ? minWithMip : minNoMip);

        // Anisotropie (falls gewünscht & verfügbar)
        if (m_Filter == Filter::Anisotropic) {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
            GLfloat maxAniso = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::max(2.0f, maxAniso)); // setzt auf Maximum
#endif
        }
        else {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
#endif
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture2D::Submit(uint8_t unit) const {
        if (!IsValid()) return;
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_Tex);
    }

} // namespace Bolt
