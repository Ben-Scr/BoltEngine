#pragma once
#include "..\Collections\Vec2.hpp"
#include "..\Collections\Color.hpp"
#include "..\Collections\Viewport.hpp"


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

    struct GLInitProperties {
        Color BackgroundColor;
        bool EnableCulling;
        GLCullingMode CullingMode;

        GLInitProperties(Color backgroundColor, bool enableCulling, GLCullingMode cullMode)
            : BackgroundColor{ backgroundColor }, EnableCulling{ enableCulling }, CullingMode{ cullMode } {
        }
    };



	class OpenGL {
	public:
		static void Initialize(const GLInitProperties& glInitProps);
	};
}