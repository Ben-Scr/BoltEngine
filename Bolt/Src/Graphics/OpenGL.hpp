#pragma once
#include "Collections\Vec2.hpp"
#include "Collections\Color.hpp"
#include "Collections\Viewport.hpp"


namespace Bolt {
	enum GLCullingMode {
		GLNone = 0,
		GLFrontLeft = 0x0400,
		GLFrontRight = 0x0401,
		GLBackLeft = 0x0402,
		GLBackRight = 0x0403,
		GLFront = 0x0404,
		GLBack = 0x0405,
		GLLeft = 0x0406,
		GLRight = 0x0407,
		GLFrontAndBack = 0x0408
	};

	struct GLInitProperties2D {
		Color BackgroundColor;
		GLCullingMode CullingMode;

		GLInitProperties2D(const Color& backgroundColor, GLCullingMode cullMode)
			: BackgroundColor{ backgroundColor }, CullingMode{ cullMode } {
		}
	};



	class OpenGL {
	public:
		OpenGL() = delete;

		static bool Initialize(const GLInitProperties2D& glInitProps);
		static bool IsInitialized() { return s_IsInitialized; }
		static void BlendFunc(GLenum sFactor, GLenum dFactor);
		static void Enable(GLenum glEnum);
		static void Disable(GLenum glEnum);
		static void CullFace(GLCullingMode cullingMode);
		static void SetBackgroundColor(const Color& backgroundColor);
		static Color GetBackgroundColor();
	private:
		static bool s_IsInitialized;
	};
}