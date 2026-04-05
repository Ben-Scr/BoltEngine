#pragma once
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"
#include "TextureHandle.hpp"
#include "Instance44.hpp"

#include <vector>

namespace Bolt {
	class Scene;

	class Renderer2D {
	public:
		void Initialize();
		void BeginFrame();
		void EndFrame();
		void Shutdown();
		void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
		bool IsEnabled() const { return m_IsEnabled; }
		bool IsInitialized() const { return m_IsInitialized; }

		// Set a custom FBO to render the next frame into instead of the default framebuffer.
		// The target is consumed and cleared after BeginFrame() executes.
		// Used by the editor viewport system so scene rendering targets the viewport panel.
		void SetOutputTarget(unsigned int fboId, int width, int height) {
			m_OutputFboId = fboId;
			m_OutputWidth = width;
			m_OutputHeight = height;
		}

		const size_t GetRenderedInstancesCount() const { return m_RenderedInstancesCount; }
		const float GetRRenderLoopDuration() const { return m_RenderLoopDuration; }

	private:
		void RenderScenes();
		void RenderScene(const Scene& scene);

		size_t m_RenderedInstancesCount = 0;
		float m_RenderLoopDuration = 0.0f;
		bool m_IsInitialized = false;
		bool m_IsEnabled = true;

		// Reused across frames to avoid per-frame heap allocation. (F-18)
		std::vector<Instance44> m_Instances;

		// Optional custom render target registered by the editor viewport. (F-08)
		unsigned int m_OutputFboId = 0;
		int m_OutputWidth = 0;
		int m_OutputHeight = 0;

		QuadMesh m_QuadMesh;
		SpriteShaderProgram m_SpriteShader;
	};
}
