#include "../pch.hpp"
#include "OpenGL.hpp"
#include <glad/glad.h>

namespace Bolt {
	bool OpenGL::Initialize(const GLInitProperties2D& glInitProps) {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			Logger::Error("OpenGL", "Glad failed to load");
			return false;
		}

		SetBackgroundColor(glInitProps.BackgroundColor);

		Enable(GL_BLEND);
		BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		CullFace(glInitProps.CullingMode);

		return true;
	}
	void OpenGL::BlendFunc(GLenum sFactor, GLenum dFactor) {
		glBlendFunc(sFactor, dFactor);
	}

	void OpenGL::Enable(GLenum glEnum){
		glEnable(glEnum);
	}
	void OpenGL::Disable(GLenum glEnum) {
		glDisable(glEnum);
	}

	void OpenGL::CullFace(GLCullingMode cullingMode) {
		if (cullingMode == GLCullingMode::GLNone) {
			glDisable(GL_CULL_FACE);
		}
		else {
			glEnable(GL_CULL_FACE);
			glCullFace(cullingMode);
		}
	}

	void OpenGL::SetBackgroundColor(const Color& backgroundColor) {
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
	}
}