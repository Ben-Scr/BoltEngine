#pragma once
#include "../Collections/Vec2.hpp"
#include <cstdint>
#include <string>

namespace Bolt {
	enum class Filter { Point, Bilinear, Trilinear, Anisotropic };
	enum class Wrap : uint32_t { Repeat = 0x2901, Clamp = 0x812F, Mirror = 0x8370, Border = 0x812D };

	class Texture2D {
	public:
		Texture2D() = default;
		// Komfort-Konstruktor: lädt sofort
		Texture2D(const char* path,
			Filter filter = Filter::Point,
			Wrap wrapU = Wrap::Clamp,
			Wrap wrapV = Wrap::Clamp,
			bool generateMipmaps = true,
			bool srgb = false,
			bool flipVertical = true);

		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) noexcept;
		Texture2D& operator=(Texture2D&&) noexcept;

		void Destroy();


		// Laden / Neu-laden
		bool Load(const char* path,
			bool generateMipmaps = true,
			bool srgb = false,
			bool flipVertical = true);

		// In den angegebenen Texture-Unit binden (0..n)
		void Submit(uint8_t unit) const;

		// Sampler-Parameter setzen (wirken auf dieses Texture-Object)
		void SetSampler(Filter filter, Wrap u = Wrap::Clamp, Wrap v = Wrap::Clamp);

		// Abfragen
		bool IsValid() const { return m_Tex != 0; }
		unsigned Handle() const { return m_Tex; }   // GL texture name (GLuint)
		Vec2 Size() const { return Vec2{ float(m_Width), float(m_Height) }; }
		float AspectRatio() const { return m_Height != 0 ? float(m_Width) / float(m_Height) : 0.0f; }

	private:
		// GL-Objekt
		unsigned m_Tex = 0;
		int m_Width = 0, m_Height = 0, m_Channels = 0;

		// gespeicherte Sampler-Wünsche
		Filter m_Filter = Filter::Point;
		Wrap   m_WrapU = Wrap::Clamp;
		Wrap   m_WrapV = Wrap::Clamp;
		bool   m_HasMips = true;
		void ApplySamplerParams() const;
	};

} // namespace Bolt
