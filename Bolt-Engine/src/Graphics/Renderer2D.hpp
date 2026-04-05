#pragma once
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"
#include "TextureHandle.hpp"
#include "Instance44.hpp"

namespace Bolt {
	class Scene;

	class Renderer2D {
	public:
		void Initialize();
		void BeginFrame();
		void EndFrame();
		void Shutdown();
		void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
		bool IsEnabled()const { return m_IsEnabled; }
		bool IsInitialized() const { return m_IsInitialized; }

		const size_t GetRenderedInstancesCount() const { return m_RenderedInstancesCount; }
		const float GetRRenderLoopDuration() const { return m_RenderLoopDuration; }

	private:
		void RenderScenes();
		void RenderScene(const Scene& scene);

		size_t m_RenderedInstancesCount;

		// Info: Duration in ms
		float m_RenderLoopDuration;

		bool m_IsInitialized;
		bool m_IsEnabled{ true };

		QuadMesh m_QuadMesh;
		SpriteShaderProgram m_SpriteShader;
	};
}
