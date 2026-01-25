#pragma once
#include "../Graphics/SpriteShaderProgram.hpp"
#include "../Graphics/QuadMesh.hpp"

namespace Bolt {
	class Scene;

	class GuiRenderer {
	public:
		void Initialize();
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		void RenderScene(const Scene& scene);

	private:
		SpriteShaderProgram m_SpriteShader;
		QuadMesh m_QuadMesh;
		bool m_IsInitialized;
	};
}