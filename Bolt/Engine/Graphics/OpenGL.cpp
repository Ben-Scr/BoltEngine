#include "../pch.hpp"
#include "OpenGL.hpp"
#include <glad/glad.h>

namespace Bolt {
	void OpenGL::Initialize(const GLInitProperties& glInitProps) {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			Logger::Error("OpenGL", "Glad failed to load");
			return;
		}

		Color c = glInitProps.BackgroundColor;
		glClearColor(c.r, c.g, c.b, c.a);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (glInitProps.EnableCulling) {
			glEnable(GL_CULL_FACE);
			glCullFace(glInitProps.CullingMode);
		}
		else {
			glDisable(GL_CULL_FACE);
		}
	}
}