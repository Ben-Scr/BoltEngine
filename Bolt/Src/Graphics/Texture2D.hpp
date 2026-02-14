#pragma once
#include "Collections/Vec2.hpp"
#include "Core/Core.hpp"
#include <cstdint>
#include <string>

namespace Bolt {
	enum class BOLT_API Filter { Point, Bilinear, Trilinear, Anisotropic };
	enum class BOLT_API Wrap : uint32_t { Repeat = 0x2901, Clamp = 0x812F, Mirror = 0x8370, Border = 0x812D };

	struct BOLT_API ImageData {
		ImageData(int width, int height, unsigned char* pixels)
			: Width(width), Height(height), Pixels(pixels) {
		}

		int Width;
		int Height;
		unsigned char* Pixels;

		void FlipVerticalRGBA(int bytesPerPixel = 4)
		{
			if (!Pixels || Width <= 0 || Height <= 0 || bytesPerPixel <= 0) return;

			const size_t rowBytes = (size_t)Width * (size_t)bytesPerPixel;
			std::vector<unsigned char> temp(rowBytes);

			for (int y = 0; y < Height / 2; ++y)
			{
				unsigned char* rowTop = Pixels + (size_t)y * rowBytes;
				unsigned char* rowBottom = Pixels + (size_t)(Height - 1 - y) * rowBytes;

				std::memcpy(temp.data(), rowTop, rowBytes);
				std::memcpy(rowTop, rowBottom, rowBytes);
				std::memcpy(rowBottom, temp.data(), rowBytes);
			}
		}

		static inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

		static inline void SampleRGBA8(
			const std::vector<unsigned char>& src, int w, int h,
			int x, int y,
			unsigned char bgR, unsigned char bgG, unsigned char bgB, unsigned char bgA,
			float& r, float& g, float& b, float& a)
		{
			if (x < 0 || y < 0 || x >= w || y >= h) {
				r = (float)bgR; g = (float)bgG; b = (float)bgB; a = (float)bgA;
				return;
			}
			const size_t i = ((size_t)y * (size_t)w + (size_t)x) * 4u;
			r = (float)src[i + 0];
			g = (float)src[i + 1];
			b = (float)src[i + 2];
			a = (float)src[i + 3];
		}

		static inline void BilinearRGBA8(
			const std::vector<unsigned char>& src, int w, int h,
			float x, float y,
			unsigned char bgR, unsigned char bgG, unsigned char bgB, unsigned char bgA,
			unsigned char& outR, unsigned char& outG, unsigned char& outB, unsigned char& outA)
		{
			// Use pixel coordinates where valid texel centers lie in [0..w-1], [0..h-1]
			const int x0 = (int)std::floor(x);
			const int y0 = (int)std::floor(y);
			const int x1 = x0 + 1;
			const int y1 = y0 + 1;

			const float tx = x - (float)x0;
			const float ty = y - (float)y0;

			float r00, g00, b00, a00, r10, g10, b10, a10, r01, g01, b01, a01, r11, g11, b11, a11;
			SampleRGBA8(src, w, h, x0, y0, bgR, bgG, bgB, bgA, r00, g00, b00, a00);
			SampleRGBA8(src, w, h, x1, y0, bgR, bgG, bgB, bgA, r10, g10, b10, a10);
			SampleRGBA8(src, w, h, x0, y1, bgR, bgG, bgB, bgA, r01, g01, b01, a01);
			SampleRGBA8(src, w, h, x1, y1, bgR, bgG, bgB, bgA, r11, g11, b11, a11);

			const float r0 = Lerp(r00, r10, tx);
			const float g0 = Lerp(g00, g10, tx);
			const float b0 = Lerp(b00, b10, tx);
			const float a0 = Lerp(a00, a10, tx);

			const float r1 = Lerp(r01, r11, tx);
			const float g1 = Lerp(g01, g11, tx);
			const float b1 = Lerp(b01, b11, tx);
			const float a1 = Lerp(a01, a11, tx);

			const float rf = Lerp(r0, r1, ty);
			const float gf = Lerp(g0, g1, ty);
			const float bf = Lerp(b0, b1, ty);
			const float af = Lerp(a0, a1, ty);

			auto clampU8 = [](float v) -> unsigned char {
				v = std::clamp(v, 0.0f, 255.0f);
				return (unsigned char)std::lround(v);
				};

			outR = clampU8(rf);
			outG = clampU8(gf);
			outB = clampU8(bf);
			outA = clampU8(af);
		}

		// Rotates an RGBA8 image by any angle (degrees).
		// - pixels: must be width * height * 4 bytes
		// - width/height: updated if expandCanvas == true (or left unchanged if false)
		// - expandCanvas: if true, grows output to fully fit rotated image; otherwise crops to original size
		// - background: fill color for areas outside the source image (default transparent)
		void SetRotationRGBAAnyAngle(
			std::vector<unsigned char>& pixels, int& width, int& height, double degrees,
			bool expandCanvas = false,
			unsigned char bgR = 0, unsigned char bgG = 0, unsigned char bgB = 0, unsigned char bgA = 0)
		{
			if (width <= 0 || height <= 0) return;
			if (pixels.size() != (size_t)width * (size_t)height * 4u) return;

			const double rad = degrees * (3.14159265358979323846 / 180.0);
			const double c = std::cos(rad);
			const double s = std::sin(rad);

			const int srcW = width;
			const int srcH = height;
			const std::vector<unsigned char> src = pixels;

			int dstW = srcW;
			int dstH = srcH;

			// Compute expanded bounding box if requested
			if (expandCanvas) {
				const double cx = (double)srcW * 0.5;
				const double cy = (double)srcH * 0.5;

				// Corners in continuous image space
				const double corners[4][2] = {
					{0.0 - cx, 0.0 - cy},
					{(double)srcW - cx, 0.0 - cy},
					{0.0 - cx, (double)srcH - cy},
					{(double)srcW - cx, (double)srcH - cy}
				};

				double minX = 1e30, minY = 1e30;
				double maxX = -1e30, maxY = -1e30;

				for (int i = 0; i < 4; ++i) {
					const double x = corners[i][0];
					const double y = corners[i][1];
					const double rx = x * c - y * s;
					const double ry = x * s + y * c;

					minX = std::min(minX, rx);
					minY = std::min(minY, ry);
					maxX = std::max(maxX, rx);
					maxY = std::max(maxY, ry);
				}

				dstW = (int)std::ceil(maxX - minX);
				dstH = (int)std::ceil(maxY - minY);

				dstW = std::max(dstW, 1);
				dstH = std::max(dstH, 1);
			}

			std::vector<unsigned char> out((size_t)dstW * (size_t)dstH * 4u, 0);

			const double srcCx = (double)srcW * 0.5;
			const double srcCy = (double)srcH * 0.5;
			const double dstCx = (double)dstW * 0.5;
			const double dstCy = (double)dstH * 0.5;

			// Inverse rotation (dest -> src) uses -rad:
			// [x']   [ c  s][x]
			// [y'] = [-s  c][y]
			const double ic = c;
			const double is = -s;

			for (int y = 0; y < dstH; ++y) {
				for (int x = 0; x < dstW; ++x) {
					// Use pixel centers for better quality
					const double dx = ((double)x + 0.5) - dstCx;
					const double dy = ((double)y + 0.5) - dstCy;

					const double sx = dx * ic + dy * (-is); // dx*c + dy*s
					const double sy = dx * is + dy * ic;    // -dx*s + dy*c

					const float srcX = (float)(sx + srcCx - 0.5);
					const float srcY = (float)(sy + srcCy - 0.5);

					unsigned char r, g, b, a;
					BilinearRGBA8(src, srcW, srcH, srcX, srcY, bgR, bgG, bgB, bgA, r, g, b, a);

					const size_t di = ((size_t)y * (size_t)dstW + (size_t)x) * 4u;
					out[di + 0] = r;
					out[di + 1] = g;
					out[di + 2] = b;
					out[di + 3] = a;
				}
			}

			pixels.swap(out);
			width = dstW;
			height = dstH;
		}

	};

	class BOLT_API Texture2D {
	public:
		Texture2D() = default;
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

		bool Load(const char* path,
			bool generateMipmaps = true,
			bool srgb = false,
			bool flipVertical = true);

		void Submit(uint8_t unit) const;


		void SetSampler(Filter filter, Wrap u = Wrap::Clamp, Wrap v = Wrap::Clamp);


		bool IsValid() const { return m_Tex != 0; }

		ImageData* GetImageData() const;
		unsigned GetHandle() const { return m_Tex; }
		Vec2 Size() const { return Vec2{ float(m_Width), float(m_Height) }; }
		float AspectRatio() const { return m_Height != 0 ? float(m_Width) / float(m_Height) : 0.0f; }

	private:
		unsigned m_Tex = 0;
		int m_Width = 0, m_Height = 0, m_Channels = 0;

		Filter m_Filter = Filter::Point;
		Wrap   m_WrapU = Wrap::Clamp;
		Wrap   m_WrapV = Wrap::Clamp;
		bool   m_HasMips = true;
		void ApplySamplerParams() const;
	};

} // namespace Bolt
